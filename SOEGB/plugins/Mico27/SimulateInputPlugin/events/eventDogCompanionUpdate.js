export const id = "EVENT_DOG_COMPANION_UPDATE";
export const name = "Update Companion Dog";
export const groups = ["EVENT_GROUP_ACTOR"];
export const fields = [
  {
    key: "actorId",
    label: "Dog Actor",
    type: "actor",
    defaultValue: "$self$",
  },
  {
    key: "attackScript",
    label: "On Dog Attack",
    description: "Runs once whenever the dog lands a bite. Add sound or an attack animation here.",
    type: "events",
  },
];

export const compile = (input, helpers) => {
  const { _addComment, _callNative, _compileSubScript, _stackPop, _stackPushConst, _stackPushReference, getActorIndex } = helpers;
  const attackScript = _compileSubScript("attack", input.attackScript || [], "dog_attack_feedback_0");
  _addComment("Run non-blocking companion dog follow and combat behavior");
  _stackPushConst(`_${attackScript}`);
  _stackPushConst(`___bank_${attackScript}`);
  _stackPushReference(getActorIndex(input.actorId || "$self$"));
  _callNative("dog_companion_update");
  _stackPop(3);
};
