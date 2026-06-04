export const id = "EVENT_THRAXX_CHEST_FRAME";
export const name = "Draw Thraxx Chest Frame";
export const groups = ["EVENT_GROUP_SCENE"];
export const fields = [
  {
    key: "frame",
    label: "Frame",
    type: "number",
    min: 0,
    max: 16,
    defaultValue: 0,
  },
];

export const compile = (input, helpers) => {
  const { _addComment, _callNative, _stackPop, _stackPushConst } = helpers;
  const frame = Math.max(0, Math.min(16, Number(input.frame || 0)));
  _addComment(`Draw Thraxx chest frame ${frame}`);
  _stackPushConst(frame);
  _callNative("thraxx_chest_draw_frame");
  _stackPop(1);
};
