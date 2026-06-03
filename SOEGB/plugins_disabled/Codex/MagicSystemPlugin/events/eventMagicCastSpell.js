export const id = "EVENT_MAGIC_CAST_SPELL";
export const name = "Magic: Cast Spell";
export const groups = ["EVENT_GROUP_SCENE"];

export const autoLabel = (fetchArg) => {
  const spellId = fetchArg("spellId");
  return `Magic: Cast Spell ${spellId}`;
};

export const fields = [
  {
    key: "spellId",
    label: "Spell ID",
    description: "Numeric spell identifier. 1 could be Storm, 2 could be Flash, and so on.",
    type: "number",
    min: 0,
    max: 255,
    defaultValue: 1,
  },
  {
    key: "targetMode",
    label: "Target Mode",
    description: "Nearest to Player is implemented first. Other modes are reserved for future cursor/menu targeting.",
    type: "select",
    options: [
      ["nearest_player", "Nearest to Player"],
      ["nearest_cursor", "Nearest to Cursor (future)"],
      ["manual_actor", "Manual Actor (future)"],
    ],
    defaultValue: "nearest_player",
  },
  {
    key: "damageVariable",
    label: "Damage Variable",
    description: "Variable where the rolled damage will be written before the target hit script runs.",
    type: "variable",
    defaultValue: "LAST_VARIABLE",
  },
  {
    key: "minDamage",
    label: "Min Damage",
    type: "number",
    min: 0,
    max: 255,
    defaultValue: 1,
  },
  {
    key: "maxDamage",
    label: "Max Damage",
    type: "number",
    min: 0,
    max: 255,
    defaultValue: 2,
  },
  {
    key: "rangePx",
    label: "Range (px)",
    description: "Maximum Manhattan distance from the player to the target.",
    type: "number",
    min: 8,
    max: 255,
    defaultValue: 112,
  },
  {
    key: "freezeFrames",
    label: "Freeze Frames",
    description: "How long the target actor logic stays disabled after the spell hits.",
    type: "number",
    min: 0,
    max: 255,
    defaultValue: 30,
  },
  {
    key: "effectReservedTiles",
    label: "Effect Actor Reserved Tiles",
    description: "The reusable hidden effect actor is found by this Reserved Tiles value.",
    type: "number",
    min: 0,
    max: 255,
    defaultValue: 23,
  },
  {
    key: "effectFrames",
    label: "Effect Visible Frames",
    description: "How long the effect actor stays visible before the update event hides it.",
    type: "number",
    min: 0,
    max: 255,
    defaultValue: 44,
  },
  {
    key: "effectYOffsetPx",
    label: "Effect Y Offset (px)",
    description: "Positive value draws the effect above the target by this many pixels.",
    type: "number",
    min: 0,
    max: 255,
    defaultValue: 24,
  },
  {
    key: "hitCollisionGroup",
    label: "Hit Collision Group",
    description: "Collision group passed to the target actor's hit script.",
    type: "collisionMask",
    includePlayer: true,
    defaultValue: "3",
  },
];

const TARGET_MODES = {
  nearest_player: 0,
  nearest_cursor: 1,
  manual_actor: 2,
};

const clampByte = (value, fallback) => {
  const parsed = Number(value);
  if (!Number.isFinite(parsed)) return fallback;
  return Math.max(0, Math.min(255, parsed | 0));
};

export const compile = (input, helpers) => {
  const {
    _addComment,
    _callNative,
    _stackPop,
    _stackPushConst,
    _stackPushReference,
    getVariableAlias,
  } = helpers;

  const spellId = clampByte(input.spellId, 1);
  const targetMode = TARGET_MODES[input.targetMode] || 0;
  const minDamage = clampByte(input.minDamage, 1);
  const maxDamage = clampByte(input.maxDamage, 2);
  const rangePx = clampByte(input.rangePx, 112);
  const freezeFrames = clampByte(input.freezeFrames, 30);
  const effectReservedTiles = clampByte(input.effectReservedTiles, 23);
  const effectFrames = clampByte(input.effectFrames, 44);
  const effectYOffsetPx = clampByte(input.effectYOffsetPx, 24);
  const hitCollisionGroup = 1 << Number(input.hitCollisionGroup || 3);

  _addComment(`Magic: cast spell ${spellId}`);

  _stackPushConst(spellId);
  _stackPushConst(targetMode);
  _stackPushConst(rangePx);
  _stackPushConst(hitCollisionGroup);
  _callNative("magic_configure_targeting");
  _stackPop(4);

  _stackPushReference(getVariableAlias(input.damageVariable));
  _stackPushConst(minDamage);
  _stackPushConst(maxDamage);
  _stackPushConst(effectReservedTiles);
  _callNative("magic_configure_damage");
  _stackPop(4);

  _stackPushConst(freezeFrames);
  _stackPushConst(effectFrames);
  _stackPushConst(effectYOffsetPx);
  _callNative("magic_configure_effect");
  _stackPop(3);

  _callNative("magic_cast_configured");
};
