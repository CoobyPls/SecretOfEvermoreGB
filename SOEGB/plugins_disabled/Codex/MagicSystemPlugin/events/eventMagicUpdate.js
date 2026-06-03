export const id = "EVENT_MAGIC_UPDATE";
export const name = "Magic: Update Timers";
export const groups = ["EVENT_GROUP_SCENE"];
export const fields = [];

export const compile = (input, helpers) => {
  const { _addComment, _callNative } = helpers;
  _addComment("Magic: update spell effect/freeze timers");
  _callNative("magic_system_update");
};
