export const id = "EVENT_MENU_HUD_NUMBERS";
export const name = "Draw Menu HUD Numbers";
export const groups = ["EVENT_GROUP_SCENE"];
export const fields = [];

export const compile = (input, helpers) => {
  const { _addComment, _callNative } = helpers;
  _addComment("Draw fixed-position HP, item counts, and talon total on Menu background");
  _callNative("menu_hud_draw_numbers");
};
