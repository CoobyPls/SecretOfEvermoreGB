export const id = "EVENT_GAMEPLAY_HUD_NUMBERS";
export const name = "Draw Gameplay HUD Numbers";
export const groups = ["EVENT_GROUP_SCENE"];
export const fields = [];

export const compile = (input, helpers) => {
  const { _addComment, _callNative } = helpers;
  _addComment("Draw stable Boy and Dog HP number tiles in the gameplay HUD window");
  _callNative("gameplay_hud_draw_numbers");
};
