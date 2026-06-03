export const id = "EVENT_STORM_CAST_NEAREST";
export const name = "Cast Storm On Nearest Enemy";
export const groups = ["EVENT_GROUP_SCENE"];
export const fields = [];

export const compile = (input, helpers) => {
  const { _addComment, _callNative } = helpers;
  _addComment("Auto-target nearest valid enemy and apply Storm damage");
  _callNative("storm_cast_nearest");
};
