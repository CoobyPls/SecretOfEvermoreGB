export const id = "EVENT_DOG_ENEMY_AGGRO_UPDATE";
export const name = "Skeeto Proximity Chase";
export const groups = ["EVENT_GROUP_ACTOR"];
export const fields = [
  {
    key: "actorId",
    label: "Enemy Actor",
    type: "actor",
    defaultValue: "$self$",
  },
];

export const compile = (input, helpers) => {
  const { _addComment, _callNative, _stackPop, _stackPushReference, getActorIndex } = helpers;
  _addComment("Skeeto chases the nearest party member only within two tiles");
  _stackPushReference(getActorIndex(input.actorId || "$self$"));
  _callNative("dog_enemy_aggro_update");
  _stackPop(1);
};
