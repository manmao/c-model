/*
 * server.h
 *
 * Copyright (C) SINA Corporation
 *
 */

#ifndef SERVER_H
#define SERVER_H

void init_log(void);
void init_server(void);
void init_signal(void);
void init_timer(void);
void init_workers(void);
void init_threads(void);
void init_processes(void);
void *process_clients(void *arg);
void do_stats(void);
void exit_cleanup(void);
void stop_threads(void);
void stop_workers(void);

#endif
