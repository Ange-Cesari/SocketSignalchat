
/**
 * @Author: CESARI Ange <ange>
 * @Email:  ange.cesari@isen.yncrea.fr
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFSIZE 512

struct struct_message
{
    int Identifiant;
    int Pid;
    char Texte[BUFSIZE];
};

struct struct_message message;
char message_recu[sizeof(message)];
int fd;
int nb_byte_lu;
char *retour_pipe;

void signal_handler(int num_sig);

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("./chat_child {nom du pipe}");
        exit(1);
    }

    retour_pipe = argv[1];

    signal(SIGINT, &signal_handler);
    signal(SIGUSR1, &signal_handler);

    printf("\t# Processus  Serveur Fils : %d \n\n", getpid()); // Identite du processus.

    // Test de connexion au pipe | Creation du pipe si inexistant
    if ((fd = open(retour_pipe, O_RDWR)) == -1) {
        perror("\t# SERVEUR : Echec Ouverture du ATOB ou pipe innexistant\n" );
        printf("\t# Erreur à l'ouverture du pipe nommé côté serveur... Tentative de creation du pipe.\n" );

        // Cree un pipe nomme en precisant ses droits et teste sa creation.
        if (mkfifo(retour_pipe, 0644) != 0) {
            perror("# Creation du ATOB\n" );
            printf("# Erreur ouverture du pipe nomme cote serveur \n");
            exit(1);
        }
    }

    //On RE-teste de se connecter vu que l'on vien tde le recreer
    if ((fd = open(retour_pipe, O_RDWR )) == -1) {
        perror("\t# SERVEUR : Ouverture du ATOB\n");
        printf("\t# Erreur ouverture du pipe nomme cote serveur\n");
        exit(1);
    }

    //On affiche le succes de la creation du pipe.
    printf("\t>> Ouverture du pipe reussie !\n\n");

    // Boucle du serveur
    while (true) {
        // Remise à zero des variable de reception
        bzero(&message_recu, sizeof(message));
        bzero(&message, sizeof(message));

        //On lit ce qui est envoye par le client.
        if ((nb_byte_lu = read(fd, &message_recu, sizeof(message_recu))) == -1)
            printf("SERVEUR : Erreur de lecture du pipe nommé %s\n", retour_pipe);
        else {
            // Affichage de la lecture
            memcpy(&message, &message_recu, sizeof(message));
            printf("SERVEUR : %d bytes lus | Taille du message: %zu | Client %d (%d): %s \n> ", nb_byte_lu, strlen(message.Texte), message.Identifiant, message.Pid, message.Texte);
        }
    }

    // Si on sort du programme par ici, on detruit le fifo
    unlink(retour_pipe);
    return 0;
}

// Gestion des signaux
void signal_handler(int num_sig) {
    printf("\nJe suis le fork du client %s, je m'eteins : %d\n", retour_pipe, num_sig);
    printf("Je m'eteins !\n");
    unlink(retour_pipe);
    exit(0);
}
