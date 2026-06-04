export const id = "EVENT_GAMEPLAY_TRY_INTERACT";
export const name = "Try Gameplay Interaction";
export const groups = ["EVENT_GROUP_INPUT", "EVENT_GROUP_SCENE"];
export const fields = [
  {
    key: "result",
    label: "Interacted Variable",
    description: "Set true when an actor in front of the player was interacted with.",
    type: "variable",
    defaultValue: "87",
  },
];

export const compile = (input, helpers) => {
  const { _addComment, _callNative, _stackPop, _stackPushReference, getVariableAlias } = helpers;
  _addComment("Try actor interaction before using equipped item");
  _stackPushReference(getVariableAlias(input.result || "87"));
  _callNative("gameplay_try_interact");
  _stackPop(1);
};
