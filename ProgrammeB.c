/**
 * @Author: CESARI Ange <ange>
 * @Email:  ange.cesari@isen.yncrea.fr
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFSIZE 512
#define SIGEOF 16666

struct struct_message {
    int Identifiant;
    int Pid;
    char Texte[BUFSIZE];
};

char path_fifo[] = "ATOB.FIFO";
int nb_client;
struct struct_message message;
char input[512];
int fd_default;
int fd;
int length_envoi;

void signal_handler(int num_sig);
void message_client(void);

int main(int argc, char** argv) {
    signal(SIGINT, &signal_handler);
    signal(SIGUSR1, &signal_handler); // Connexion refusée par le serveur
    signal(SIGUSR2, &signal_handler); // Quand le serveur se coupe, il le dit au client connecté

    // Format de commande
    if (argc != 2) {
        printf("Commande invalide : %s {n° Client} (Merci de préciser le numéro du client)\n", argv[0]);
        printf("Un seul client maximum autorisé (sinon rejeté par le serveur) ex: ./ProgrammeB 1 (ou n)\n\n");
        exit(1);
    }
    
    nb_client = atoi(argv[1]);
    printf("--- Processus  Client %d : %d ---\n\n", nb_client, getpid()); // Identite du processus.

    // Test de connexion au pipe
    if ((fd_default = open(path_fifo, O_WRONLY)) == -1) {
        perror ( "# CLIENT : Ouverture du ATOB\n" );
        printf ( "# Erreur ouverture du pipe nommé du côté client\n" );
        exit(1);
    }

    // Preparation du message de demande de connexion
    message.Identifiant = nb_client;
    message.Pid = getpid();
    sprintf(message.Texte, "RETOUR_PIPE %d", nb_client);

    // Envoi de la demande de connexion
    if (write(fd_default, &message, sizeof(message)) == -1)
        printf("# Echec de l'envoi du message de demarrage\n");
    else {
        printf("# CLIENT : Envoie de demande de connexion...\n");
        sleep(2);

        printf("# CLIENT : Connexion acceptée !\n");
        printf("Commencez à écrire et appuyez sur Entrer pour envoyer le message \n> ");
        message_client();
    }

    return 0;
}

// Gestion de la seconde partie, une fois le fils du serveur creer etc...
void message_client(void) {
    // Test de connexion au pipe avec le fils du serveur
    if ((fd = open(message.Texte , O_WRONLY)) == -1) {
        perror ( "# CLIENT : Ouverture du pipe de communication avec le fils du serveur \n" );
        printf ( "# Erreur ouverture du pipe nommé du côté client\n" );
        exit(1);
    }

    //Test d'envoie sur le retour_pipe
    strcpy(message.Texte, "Nouveau client sur le bon pipe de communication !\n");
    if ((length_envoi = write(fd, &message, sizeof(message))) == -1) { //On envoie le message.
        printf("#CLIENT : Erreur d'écriture dans le pipe nomme 'ATOB'\n");
        exit(1);
    }

    while (true) {
        // On lit l'input de 511 caractères  maximum pour ne pas overflow le buffer
        if (scanf("%511s", input) == -1){
        
            signal_handler(SIGEOF);
		     }
        else {
            // Formatage du message à envoyer
            strcpy(message.Texte, input);

            if ((length_envoi = write(fd, &message, sizeof(message))) == -1 ) // On envoie le message.
                printf("#CLIENT : Erreur lors de l'écriture dans le pipe nommé 'ATOB'\n");
            else
                printf("#CLIENT : Message de %d bits envoyé | Taille du message (nb caractères): %zu | Message envoyé : %s\n> ", length_envoi, strlen(message.Texte), input);
        }
    }
}

  // Gestion des signaux
void signal_handler(int num_sig) {
    switch (num_sig) {
        case SIGUSR2:
            printf("La connexion avec le serveur a été interrompue ! \n");
            exit(3);
        case SIGUSR1:
            printf("La demande de connexion a été refusée par le serveur ! \n");
            exit(2);
        case SIGEOF:
            printf("\nJ'ai reçu un signal d'interruption : EOF (CTRL+D)");
            printf("# Je m'éteins... \n" );
        case SIGINT:
            printf("\nJ'ai reçu un signal d'interruption : %d\n", num_sig);
            printf("# Je m'éteins... \n" );
        

            message.Identifiant = nb_client;
            message.Pid = -1;
            sprintf(message.Texte, "RETOUR_PIPE %d", nb_client);

            if ((length_envoi = write(fd_default, &message, sizeof(message))) == -1) // On envoie le message.
                printf("#CLIENT : Erreur Warning du serveur de déconnexion du client\n");
            exit(1);
    }
}
