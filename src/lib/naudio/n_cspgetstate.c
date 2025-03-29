#include <libaudio.h>
#include "n_libaudio.h"

int n_alCSPGetState(N_ALCSPlayer *seqp)
{
	return seqp->state;
}
