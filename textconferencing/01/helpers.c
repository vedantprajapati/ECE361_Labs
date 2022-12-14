#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "helpers.h"

void convert_client_input_to_packet(char *client_input, struct message *msg)
{
	char *packet_type = strtok(client_input, ":");
	msg->type = atoi(packet_type);

	char *packet_size = strtok(NULL, ":");
	msg->size = atoi(packet_size);

	char *packet_source = strtok(NULL, ":");
	strcpy((char *)msg->source, packet_source);

	char *packet_data = strtok(NULL, ":");
	strcpy((char *)msg->data, packet_data);
}

void display_packet(struct message *msg)
{
	printf("msg->type: %d, msg->size: %d, msg->source: %s, msg->data: %s\n", msg->type, msg->size, msg->source, msg->data);
}

void display_message(char *buffer, long int type, int length, char *sender, char *text)
{
	sprintf(buffer, "%ld:%d:%s:%s", type, length, sender, text);
}

struct user *lookup_user_name(struct users *users, char *name)
{
	for (size_t i = 0; i < users->len; i++)
	{
		if (strcmp(users->array[i].username, name) == 0)
		{
			return &users->array[i];
		}
	}
	return NULL;
}

void add_user_id(struct users *users, struct user *new_user)
{
	if (users->len == users->capacity)
	{
		users->capacity *= 2;
		users->array = realloc(users->array, users->capacity * sizeof(struct user));
	}
	users->array[users->len++] = *new_user;
}

void rm_user_id(struct users *users, char *name)
{
	size_t i = 0;
	for (; i < users->len; i++)
	{
		if (strcmp(users->array[i].username, name) == 0)
		{
			break;
		}
	}

	if (i == users->len)
	{
		return;
	}

	for (size_t j = i + 1; j < users->len; j++)
	{
		memcpy(&users->array[j - 1], &users->array[j], sizeof(struct user));
	}
}

struct user *lookup_user_creds(char *usr_name, char *database)
{
	char client[32];
	char client_pass[32];

	FILE *fp = fopen(database, "r");

	if (fp == NULL)
	{
		printf("Error opening file\n");
		return NULL;
	}
	else
	{
		int readline;
		while (true)
		{
			readline = fscanf(fp, "username: %s password: %s\n", client, client_pass);
			if (strcmp(usr_name, client) == 0)
			{

				struct user *user = malloc(sizeof(struct user));
				strcpy(user->password, client_pass);
				strcpy(user->username, client);
				memset(user->session_id, 0, 32);

				fclose(fp);
				return user;
			}
			if (readline == -1)
			{
				break;
			}
		}
		fclose(fp);
		return NULL;
	}
}

void rm_session(struct sessions *sessions, struct session *session)
{
	struct sessions *past = NULL;
	struct sessions *ptr = sessions;
	while (ptr != NULL)
	{
		if (strcmp(ptr->session->id, session->id) == 0)
		{
			if (past == NULL)
			{
				sessions = ptr->next;
				free(ptr);
				ptr = sessions;
			}
			else
			{
				past->next = ptr->next;
				free(ptr);
				ptr = past->next;
			}
		}
		else
		{
			past = ptr;
			ptr = ptr->next;
		}
	}
}

struct session *add_session(struct sessions *sessions, char *session_name)
{
	struct session *new_session = malloc(sizeof(struct session));
	strcpy(new_session->id, session_name);
	new_session->activeUsers = 1;


	if (sessions)
	{
		struct sessions *past = sessions;
		sessions->session = new_session;
		sessions->next = past;
		return new_session;
	}
	else
	{
		sessions->session = new_session;
		sessions->next = NULL;
		return new_session;
	}
}

struct session *lookup_session(struct sessions *sessions, char *id)
{
	struct sessions *session_ptr = sessions;
	while (session_ptr != NULL)
	{
		if (strcmp(session_ptr->session->id, id) == 0)
		{
			return session_ptr->session;
		}
		else
		{
			session_ptr = session_ptr->next;
		}
	}
	return NULL;
}

bool add_user_to_session(struct users *users, struct sessions *sessions, struct session *session, struct user *user)
{
	if (user == NULL)
	{
		printf("user not found\n");
		return false;
	}
	else if (strcmp(user->session_id, session->id) == 0)
	{
		printf("user exists in session\n");
		return false;
	}
	else if (session == NULL)
	{
		strcpy(user->session_id, session->id);
		session = add_session(sessions, session->id);
		return true;
	}
	else
	{
		session->activeUsers++;
		strcpy(user->session_id, session->id);
		return true;
	}
}

bool rm_user_from_session(struct users *users, struct sessions *sessions, struct session *session, struct user *user)
{

	if (session == NULL)
	{
		printf("session not found\n");
		return false;
	}
	else if (user == NULL)
	{
		printf("user not found\n");
		return false;
	}
	else
	{
		session->activeUsers--;
		if (session->activeUsers == 0)
		{
			rm_session(sessions, session);
		}
		memset(user->session_id, 0, 32);
		return true;
	}
}