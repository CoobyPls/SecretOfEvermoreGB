export const id = "EVENT_GAMEPLAY_HUD_ICONS";
export const name = "Draw Gameplay HUD Item Icons";
export const groups = ["EVENT_GROUP_SCENE"];
export const fields = [];

export const compile = (input, helpers) => {
  const { _addComment, _callNative, _stackPop, _stackPush, getVariableAlias } = helpers;
  _addComment("Draw full-size equipped item graphics in gameplay HUD slots");
  _stackPush(getVariableAlias("47"));
  _stackPush(getVariableAlias("48"));
  _callNative("gameplay_hud_draw_icons");
  _stackPop(2);
};
