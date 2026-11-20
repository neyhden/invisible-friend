#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netdb.h>

#define SMTP_OPENSSL
#include "smtp.h"

#define MAIL_SERVER              "smtp.gmail.com"
#define MAIL_PORT                "587"
#define MAIL_CONNECTION_SECURITY SMTP_SECURITY_STARTTLS
#define MAIL_FLAGS               SMTP_NO_CERT_VERIFY /* Do not verify cert. */
#define MAIL_CAFILE              NULL
#define MAIL_AUTH                SMTP_AUTH_PLAIN

struct Person {
	char name[64];
	char mail[64];
};

int count_lines_in_file(FILE* file);
void generate_permutation(int* permutation, int size);
void print_int_array(const int* array, int size);

int main(int argc, char** argv) {
	srand(time(NULL));
	if (argc != 3) {
		puts("Must be called with 2 arguments.\n./a.out [CONFIG] [MAILS]");
		return 1;
	}
	char* config_path = argv[1];
	char* mails_path = argv[2];

	// Read config
	FILE* config_file = fopen(config_path, "r");
	if (config_file == NULL) {
		printf("File \"%s\" doesn't exist. Exiting...", config_path);
		return 1;
	}
	char sender[64];
	char password[20];
	char subject[128];
	char body_format[256];
	fscanf(config_file, "%[^\n]\n%[^\n]\n%[^\n]\n%[^\n]", sender, password, subject, body_format);
	fclose(config_file);

	// Read mails file
	FILE* mails_file = fopen(mails_path, "r");
	if (mails_file == NULL) {
		printf("File \"%s\" doesn't exist. Exiting...", mails_path);
		return 1;
	}
	int entries = count_lines_in_file(mails_file);
	struct Person* persons = malloc(sizeof(struct Person) * entries);
	for (int person_index = 0; person_index < entries; person_index++) {
		fscanf(mails_file, "%s %s", persons[person_index].name, persons[person_index].mail);
	}
	fclose(mails_file);

	// Send mails
	int* permutation = calloc(entries, sizeof(int));
	generate_permutation(permutation, entries);

	struct smtp *smtp;
	enum smtp_status_code rc;
	puts("Configuring...");
	rc = smtp_open(MAIL_SERVER, MAIL_PORT, MAIL_CONNECTION_SECURITY, MAIL_FLAGS, MAIL_CAFILE, &smtp);
	rc = smtp_auth(smtp, MAIL_AUTH, sender, password);
	puts("Done");

	for (int entry = 0; entry < entries; entry++) {
		char body[320];
		sprintf(body, body_format, persons[permutation[entry]].name);

		printf("Sending mail %d/%d...\n", entry + 1, entries);
		rc = smtp_address_add(smtp, SMTP_ADDRESS_FROM, sender, "Amigo invisible");
		rc = smtp_address_add(smtp, SMTP_ADDRESS_TO, persons[entry].mail, persons[entry].name);
		rc = smtp_header_add(smtp, "Subject", subject);
		rc = smtp_mail(smtp, body);
		smtp_address_clear_all(smtp);
		smtp_header_clear_all(smtp);
		puts("Done");
	}

	rc = smtp_close(smtp);
	if(rc != SMTP_STATUS_OK){
		fprintf(stderr, "smtp failed: %s\n", smtp_status_code_errstr(rc));
		return 1;
	}

	puts("Finished. Quitting...");
	free(permutation);
	return 0;
}

int count_lines_in_file(FILE* file) {
	int previous_cursor = ftell(file);
	fseek(file, 0, SEEK_SET);
	const int CHARS_TO_READ = 64;
	int total_lines = 0;
	char* file_buffer = malloc(sizeof(char) * CHARS_TO_READ);
	int read_chars = 0;
	do {
		read_chars = fread(file_buffer, sizeof(char), CHARS_TO_READ, file);
		for (int char_index = 0; char_index < read_chars; char_index++) {
			if (file_buffer[char_index] == '\n') {
				total_lines++;
			}
		}

	} while (read_chars == CHARS_TO_READ);
	free(file_buffer);
	fseek(file, previous_cursor, SEEK_SET);
	return total_lines;
}

void generate_permutation(int* permutation, int size) {
	int* used_items = calloc(size, sizeof(int));
	int* available_items = calloc(size, sizeof(int));

	do {
		memset(used_items, 0, sizeof(int) * size);
		memset(available_items, 0, sizeof(int) * size);
		memset(permutation, 0, sizeof(int) * size);
		for (int current_index = 0; current_index < size; current_index++) {
			int available_item_index = 0;
			for (int item = 0; item < size; item++) {
				if (current_index == item) continue;
				if (used_items[item]) continue;
				available_items[available_item_index] = item;
				available_item_index++;
			}
			int available_items_size = size - current_index - 1 + used_items[current_index];
			int random_index = (available_items_size == 0) ? 0 : (rand() % available_items_size);
			used_items[available_items[random_index]] = 1;
			permutation[current_index] = available_items[random_index];
		}
	} while (!used_items[size - 1]);

	free(used_items);
	free(available_items);
}

void print_int_array(const int* array, int size) {
	printf("[ ");
	for (int index = 0; index < size; index++) {
		printf("%d ", array[index]);
	}
	printf("]");
}
