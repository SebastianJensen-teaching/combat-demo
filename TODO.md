# GENERAL
- [ ] Render everything to canvas and scale it with window size
    - [ ] Fullscreen toggling

# GAMEPLAY:
- [ ] Activate characters
    - [ ] Melee attacks
    - [ ] Ranged attacks
        - [ ] Are there bullets in clip?
        - [ ] Is target in range?
        - [ ] Is target in LoS?
        - [ ] If there is a gun jam defect, determine gun jam
        - [ ] Roll over proficiency to hit
            - [ ] On crit miss, check 2d6 <= quality
            - [ ] If quality fails give weapon one random defect:
                - [ ] Clip size - 1
                - [ ] Broken burst switch
                - [ ] Gun jams 20% of time
                - [ ] Doubled reload cost
                - [ ] -20% to hit
            - [ ] If there are more defects than floor(quality / 2), weapon explodes  
        - [ ] Remaining armor = target armor - armor penetration
        - [ ] Target hitpoints -= (ranged based damage - rem. armor)
        - [ ] Damage based on range 
    - [ ] Other actions
         - [ ] Hunker down
         - [ ] Aim shot
         - [ ] Reload
         - [ ] Burst switch
    - [X] Card Activation Combos
    - [X] Cards cannot activate Commander
    - [X] Prematurely end character activation phase
        - [ ] Reactivated cards get only one action
- [ ] Card use effects
- [ ] Victory + defeat conditions

# UI:

- [ ] Activated character Action Point Pips

- [ ] End turn / phase button animations
- [ ] Better ui feedback
- [ ] Toggle buttons for command deck list and discard pile list
- [ ] Command deck list UI
- [ ] Discard deck list UI
    - [ ] Mouse over in list shows card preview