#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "LinkedList.h"
#include "Channel.h"

#define MAX 100
#define MAX_UTIL 0.9

int n = 0;
int **constraint = NULL;
int *vars = NULL;
struct LinkedList **domains = NULL;

int** read_topo_from_file(char* filename);
int* read_channels_from_file(char* filename);
void print_topo(int n, int** constraint);
void print_channel_utilization(int n, struct LinkedList **domains);
struct Channel get_current_channel(int index);

void node_consistent();

int main(){
	srand((time(0)));
	int i;
	constraint = read_topo_from_file("topo.txt");
	
	vars = read_channels_from_file("channels.txt");

	domains = malloc(n*sizeof(struct LinkedList*));
	for (i = 0; i < n; i++){
		domains[i] = newLinkedList();
		//vars[i] = malloc(sizeof(struct Channel));
		/* generate channel utilization randomly */
		/* total_util: [0.3; MAX_UTIL]
		 * env_util: [0.3; total_util] 
		 * 0.3 is the minimum environment ultilization*/
		float a = MAX_UTIL - 0.3;
		int j, c = 1;
		for (j = 0; j < 3; j++){
			struct Channel channel;
			channel.chan_no = c;
			channel.total_util = ((float)rand()/(float)(RAND_MAX)) * a + 0.3;
			channel.env_util = ((float)rand()/(float)(RAND_MAX)) * (channel.total_util - 0.3) + 0.3;
			c += 5;
			struct Node *node = newNode((void*)&channel, CHANNEL_TYPE);
			push(domains[i], node);
		}
	}


	//print_topo(n, constraint);
	printf("channel utilization before performing node_consistent\n");
	print_channel_utilization(n, domains);
	node_consistent();
	printf("channel utilization after performing node_consistent\n");
	print_channel_utilization(n, domains);
	return 0;
}

int** read_topo_from_file(char* filename){
	FILE *file = fopen(filename, "r");
	fscanf(file, "%d", &n);
	int **constraint = NULL;
	int i;
	constraint = malloc(n*sizeof(int*));
	for (i = 0; i < n; i++){
		constraint[i] = malloc(n*sizeof(int));
	}

	for (i = 0; i < n; i++){
		int j;
		for (j = 0; j < n; j++){
			fscanf(file, "%d", &constraint[i][j]);
		}
	}
	fclose(file);

	return constraint;
}

int* read_channels_from_file(char *filename){
	FILE *file = fopen(filename, "r");
	int *channel = malloc(n*sizeof(int));
	int i;
	for (i = 0; i < n; i++){
		fscanf(file, "%d", &channel[i]);
	}
	fclose(file);
	return channel;
}

void print_topo(int n, int** constraint){
	int i;
	for (i = 0; i < n; i++){
		int j;
		for (j = 0; j < n; j ++){
			printf("%-3d ", constraint[i][j]);
		}
		printf("\n");
	}
}

void print_channel_utilization(int n, struct LinkedList **domains){
	int i;
	for (i = 0; i < n; i++){
		struct Node *it = domains[i]->head;
		for (; it != NULL; it = it->next){
			struct Channel channel = *(struct Channel*)it->data;
			printf("(%f, %f) ", channel.total_util, channel.env_util);
		}
		printf("\n");
	}
}

struct Channel get_current_channel(int index){
	int chan_no = vars[index];
	struct Node *it = domains[index]->head;
	for (; it != NULL; it = it->next){
		struct Channel *chan = (struct Channel*)it->data;
		if (chan->chan_no == chan_no)
			return *chan;
	}
	struct Channel new_chan;
	new_chan.chan_no = 0;
	return new_chan;
}

void node_consistent(){
	printf("performing node_consistent\n");
	int i;
	for (i = 0; i < n; i ++){
		struct Channel cur_chan = get_current_channel(i);
		float current_bss_util = cur_chan.total_util - cur_chan.env_util;
		printf("%d, %f, %f, %f\n", cur_chan.chan_no, cur_chan.total_util, cur_chan.env_util, current_bss_util);
		struct Node *it = domains[i]->head;
		int remove_head = 0;
		for (; it != NULL; it = it->next){
			struct Channel *chan = (struct Channel*)it->data;
			if (current_bss_util + chan->env_util >= MAX_UTIL){
				/* remove this channel from the domain */
				if (it == domains[i]->head)
					remove_head = 1;
				else {
					it = it->prev;
					removeNode(domains[i], it->next);
				}
			}
		}
		if (remove_head)
			pop(domains[i]);
	}
}
