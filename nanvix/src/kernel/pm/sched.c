/*
 * Copyright(C) 2011-2016 Pedro H. Penna   <pedrohenriquepenna@gmail.com>
 *              2015-2016 Davidson Francis <davidsondfgl@hotmail.com>
 *
 * This file is part of Nanvix.
 *
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nanvix/clock.h>
#include <nanvix/const.h>
#include <nanvix/hal.h>
#include <nanvix/pm.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Schedules a process to execution.
 *
 * @param proc Process to be scheduled.
 */
PUBLIC void sched(struct process *proc)
{
	proc->state = PROC_READY;
	proc->counter = 0;
}

/**
 * @brief Stops the current running process.
 */
PUBLIC void stop(void)
{
	curr_proc->state = PROC_STOPPED;
	sndsig(curr_proc->father, SIGCHLD);
	yield();
}

/**
 * @brief Resumes a process.
 *
 * @param proc Process to be resumed.
 *
 * @note The process must stopped to be resumed.
 */
PUBLIC void resume(struct process *proc)
{
	/* Resume only if process has stopped. */
	if (proc->state == PROC_STOPPED)
		sched(proc);
}

/* Pseudo code lottery */
	/*
	nb_ticket = 0;
	ticket_tab = malloc;

	for	p in process
		si p.nice == 0
			nice = un nb entre 0 et 100
		for i de nb_ticket a (nb_ticket + nice) :
			ticket_tab[i] = p.PID;
		end for
		nb_ticket += p.nice;
	end for

	winner = rand de 0 a nb_ticket

	for 
		if p.PID == ticket_tab[winner]
			next = p;
	end for

	free ticket tab

	switch process
	*/

/**
 * @brief Yields the processor.
 */
PUBLIC void yield(void)
{
	struct process *p;	  /* Working process.     */
	struct process *next; /* Next process to run. */
	/* Re-schedule process for execution. */
	if (curr_proc->state == PROC_RUNNING)
		sched(curr_proc);

	/* Remember this process. */
	last_proc = curr_proc;

	/* Check alarm. */
	for (p = FIRST_PROC; p <= LAST_PROC; p++)
	{
		/* Skip invalid processes. */
		if (!IS_VALID(p))
			continue;

		/* Alarm has expired. */
		if ((p->alarm) && (p->alarm < ticks))
			p->alarm = 0, sndsig(p, SIGALRM);
	}

	/* Choose a process to run next. */

	int nb_ticket = 0;
	int *ticket_tab = (int *)malloc(sizeof(int));
	
	// for setting the seed of rand
	srand(1);

	for (p = FIRST_PROC; p <= LAST_PROC; p++)
	{
		/* Skip non-ready process. */
		if (p->state != PROC_READY)
			continue;

		// give a random number of ticket to process with no ticket
		if (p->nice > 100)
		{
			p->nice = rand() % 101;
		}
		
		// update the ticket_tab
		for (int i = nb_ticket ; i < (nb_ticket + p->nice); i++)
		{
			ticket_tab[i] = getpid();
		}

		nb_ticket += p->nice;
	}

	// choose a winner
	int winner = rand() % nb_ticket;

	// find the winner
	for (p = FIRST_PROC; p <= LAST_PROC; p++)
	{
		if (getpid() == ticket_tab[winner])
		{
			next = p;
		}
	}

	// free the tab
	free(ticket_tab);

	/* Switch to next process. */
	next->priority = PRIO_USER;
	next->state = PROC_RUNNING;
	next->counter = PROC_QUANTUM;
	if (curr_proc != next)
		switch_to(next);
}