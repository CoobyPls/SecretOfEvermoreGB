export const id = "EVENT_THRAXX_CHEST_TICK";
export const name = "Tick Thraxx Chest Animation";
export const groups = ["EVENT_GROUP_SCENE"];
export const fields = [];

export const compile = (input, helpers) => {
  const { _addComment, _callNative } = helpers;
  _addComment("Tick Thraxx chest background animation");
  _callNative("thraxx_chest_tick");
};
