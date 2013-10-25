#ifndef __CONN_H
#define __CONN_H

#include <stdio.h>

#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>

int	conn_insert (int);
void	conn_upkeep (fd_set *, fd_set *);
int	conn_cullselect (fd_set *, fd_set *);

#endif

