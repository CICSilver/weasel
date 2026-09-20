#include "stdafx.h"
#include "WeaselTSF.h"
#include "CandidateList.h"
#include "ResponseParser.h"

STDMETHODIMP WeaselTSF::DoEditSession(TfEditCookie ec) {
  // get commit string from server
  std::wstring commit;
  weasel::Config config;
  auto context = std::make_shared<weasel::Context>();
  weasel::ResponseParser parser(&commit, context.get(), &_status, &config,
                                &_cand->style());

  bool ok = m_client.GetResponseData(std::ref(parser));

  _UpdateLanguageBar(_status);

  bool compositionEnded = false;
  if (ok) {
    compositionEnded = false;
    if (!commit.empty()) {
      /* A caret anchor in the commit string asks for the insertion point to
         land inside the committed text instead of after it. Strip every
         anchor so none reaches the document, and remember how far back the
         caret has to travel from the end. */
      size_t anchor = std::wstring::npos;
      for (size_t pos = commit.find(WEASEL_CARET_ANCHOR);
           pos != std::wstring::npos;
           pos = commit.find(WEASEL_CARET_ANCHOR, pos)) {
        if (anchor == std::wstring::npos)
          anchor = pos;
        commit.erase(pos, 1);
      }
      const LONG caretBack = (anchor == std::wstring::npos)
                                 ? 0
                                 : static_cast<LONG>(commit.length() - anchor);

      // For auto-selecting, commit and preedit can both exist.
      // Commit the old TSF composition. If Rime immediately has a new
      // preedit (top-word input), _EndComposition() drops the local pointer
      // synchronously, so the following state check starts a new TSF
      // composition instead of observing the old one.
      if (!_IsComposing()) {
        _StartComposition(_pEditSessionContext,
                          _fCUASWorkaroundEnabled && !config.inline_preedit);
      }
      _InsertText(_pEditSessionContext, commit, caretBack);
      // Keep the candidate UI alive while the replacement composition is
      // being created; otherwise the key-down path destroys the old window
      // and the new one cannot be positioned until key-up.
      _EndComposition(_pEditSessionContext, false, !_status.composing);
      /* Ending the composition can move the insertion point back to the end
         of the committed text, so the caret is placed in its own edit session
         queued after that one. */
      if (caretBack > 0)
        _MoveCaretBack(_pEditSessionContext, caretBack);
      compositionEnded = true;
      _committed = TRUE;
    } else {
      _committed = FALSE;
    }
    if (_status.composing && (compositionEnded || !_IsComposing())) {
      _StartComposition(_pEditSessionContext,
                        _fCUASWorkaroundEnabled && !config.inline_preedit);
    } else if (!_status.composing && _IsComposing()) {
      _EndComposition(_pEditSessionContext, true);
    }
    if (_IsComposing() && config.inline_preedit) {
      _ShowInlinePreedit(_pEditSessionContext, context);
    }
  }

  if (ok && !compositionEnded)
    _UpdateCompositionWindow(_pEditSessionContext);
  // Keep the existing candidate window alive during top-word input, but
  // publish the new candidates in this key-down edit session. Positioning is
  // still updated by the queued read session after the new composition is
  // created.
  _UpdateUI(*context, _status);

  return TRUE;
}
