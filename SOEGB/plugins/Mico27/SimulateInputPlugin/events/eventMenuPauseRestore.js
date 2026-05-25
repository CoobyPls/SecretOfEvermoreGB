export const id = "EVENT_MENU_PAUSE_RESTORE";
export const name = "Restore Gameplay After Menu";
export const groups = ["EVENT_GROUP_SCENE"];
export const fields = [];

export const compile = (input, helpers) => {
  const { _addComment, _callNative } = helpers;
  _addComment("Restore gameplay state after closing Start Menu");
  _callNative("menu_pause_request_restore");
};
