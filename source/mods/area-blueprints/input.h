#ifndef MOONLAB_BLUEPRINT_INPUT_H
#define MOONLAB_BLUEPRINT_INPUT_H
/* Sample on every observed update, including world-reset and gated frames.
 * The feature owns context/authority; this helper only consumes action edges. */
typedef struct {MoonInputState survey,leave,first,second,slot,rebuild;} BlueprintInput;
static inline BlueprintInput blueprint_input(int allowed){
    BlueprintInput input;
    input.survey=moon_input_action("area-blueprints.survey","Alt+B",allowed);
    input.leave=moon_input_action("area-blueprints.leave","Escape",allowed);
    input.first=moon_input_action("area-blueprints.first-corner","F6",allowed);
    input.second=moon_input_action("area-blueprints.save-corner","F7",allowed);
    input.slot=moon_input_action("area-blueprints.next-slot","F8",allowed);
    input.rebuild=moon_input_action("area-blueprints.rebuild","Ctrl+R",allowed);
    return input;
}
#endif
