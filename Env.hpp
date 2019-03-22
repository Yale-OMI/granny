#include <stdio.h>

enum envState {
	env_idle = 0,
	env_attack,
	env_decay,
	env_sustain,
	env_release
}

class Env
{
public:
	/*
	 * constructs a default envelope... something simple
	 */
	Env();

	/*
	 * constructs envelope based on array of pts and times
	 * using a designated interpolation method
	 *
	 * @param pts - an array of n amps
	 * @param tms - an array of n-1 times
	 * @param ipt - an int (have MACROS)
	 */
	Env(float* pts, float* tms, int ipt);


private:
	int interpolation = 0; // to correspond with LINEN

}
