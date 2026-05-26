export const id = "EVENT_NEW_GAME_RESET_STATE";
export const name = "Reset New Game State";
export const groups = ["EVENT_GROUP_SCENE"];
export const fields = [];

export const compile = (input, helpers) => {
  const { _addComment, _callNative } = helpers;
  _addComment("Clear persistent run state before starting a fresh game");
  _callNative("new_game_reset_state");
};
