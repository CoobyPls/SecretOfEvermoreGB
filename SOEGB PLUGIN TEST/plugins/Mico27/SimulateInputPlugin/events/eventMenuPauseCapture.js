export const id = "EVENT_MENU_PAUSE_CAPTURE";
export const name = "Capture Gameplay Before Menu";
export const groups = ["EVENT_GROUP_SCENE"];
export const fields = [];

export const compile = (input, helpers) => {
  const { _addComment, _callNative } = helpers;
  _addComment("Capture gameplay state before opening Start Menu");
  _callNative("menu_pause_capture");
};
