#include "n_synthInternals.h"

void n_alSynAddPlayer(ALPlayer *client)
{
	client->samplesLeft = n_syn->curSamples;

	client->next = n_syn->head;
	n_syn->head = client;
}

void n_alSynAddSndPlayer(ALPlayer *client)
{
	client->samplesLeft = n_syn->curSamples;

	client->next = n_syn->head;
	n_syn->head = client;
}

void n_alSynAddSeqPlayer(ALPlayer *client)
{
	client->samplesLeft = n_syn->curSamples;

	client->next = n_syn->head;
	n_syn->head = client;
}
