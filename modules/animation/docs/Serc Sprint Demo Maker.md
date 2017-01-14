# Startup meeting

Tino, Erik, Robin, Peter, Ingrid, Daniel

## Goals
	* Museum installation tool
	* Presentation tool
	* Non linear stories
	* Spetial Action
		* Annotations
		* Stop/Label/Pause/Break/Goto/Jump
		* Run Script
		* Sub timelines
	* Interpolation functions for various properties
	* Timeline editor
	* Data structues	

## Animation timeline
	* List of param
 	* time line
	* parameter values editor
	* Interpolatin types
	* Easing
	* Play/pause/break
	* enable/disable

## Track types
	* Property
	* Composite
	* Python
	* Time control (pause break jump)

## Data Structures
	* Serializable
	* keypoints
	* interpolation
	* easing
	* Composite properties
	* discrete values
	* 

## Class for front-end gui to keep state
Animator
    Animation* currentAnim
    float time
    bool playing

    play()
    pause()
    stop()

AnimationManager
    Animation[] : Serializable
        Track[]
            Sequence[2..N]
                Keyframe[]
                    Value
                    Time

            eval(t_c, t_n)
        eval(t_c, t_n)

Animation SpecialTrack (Timemodifying, GOTO, PAUSE, PYTHON)
Animation PropertyTrack

