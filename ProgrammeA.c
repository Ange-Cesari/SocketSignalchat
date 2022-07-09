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

//Changer le MAX CLIENT si on veut plus de clients capables de se connecter (9 max)
#define BUFSIZE 512
#define MAX_CLIENT 1
#define DEFAULT_PIPE "ATOB.FIFO"

struct struct_message {
    int Identifiant;
    int Pid;
    char Texte[BUFSIZE];
};

struct struct_message message;
char message_recu[sizeof(message)];
int fd;
int nb_byte_lu;
// Liste des clients et des PIDs associés
int clients[9] = {0}; 
int fork_pid[9] = {0};

void signal_handler(int num_sig);
int nbConnectedClients();
int isConnectedClients();
void disconnectAllClients();

int main(int argc, char** argv) {
    signal(SIGINT, &signal_handler);
    printf("----------- Processus  Serveur : %d -----------\n\n", getpid()); 
    // Test de connexion au pipe et creation du pipe si inexistant
    if ((fd = open(DEFAULT_PIPE, O_RDWR)) == -1) {
        perror("# SERVEUR : Echec Ouverture du ATOB ou pipe innexistant");
        printf("# Erreur à l'ouverture du pipe nommé du côté serveur... Tentative de création du pipe.\n");

        // Crée un pipe nommé en précisant ses droits et teste sa création
        if (mkfifo(DEFAULT_PIPE, 0644) != 0) {
            perror("# Creation du ATOB");
            printf("# Erreur à l'ouverture du pipe nommé côté serveur \n");
            exit(1);
        }
    }
    //On test à nouveau de se connecter après la création
    if ((fd = open(DEFAULT_PIPE, O_RDWR )) == -1) {
        perror("# SERVEUR : Ouverture du ATOB\n");
        printf("# Erreur ouverture du pipe nomme cote serveur\n");
        exit(1);
    }
    printf("----------- Création du pipe en cours... -----------\n");
    printf("----------- Ouverture du pipe reussie ! -----------\n\n");
    // Boucle serveur
    while (true) {
        // Remise à zero des variable en reception
        bzero(&message_recu, sizeof(message));
        bzero(&message, sizeof(message));

        //lecture de ce qui est envoyé par le client
        if ((nb_byte_lu = read(fd, &message_recu, sizeof(message_recu))) == -1)
            printf("SERVEUR : Erreur de lecture du pipe nommé %s\n", DEFAULT_PIPE);
        else {
            // Affichage de cette lecture
            memcpy(&message, &message_recu, sizeof(message));
            printf("#SERVEUR : %d bytes lus | Réception demande de connexion : Client %d (%d): %s : CONNEXION ", nb_byte_lu,
                    message.Identifiant, message.Pid, message.Texte);

            if (message.Pid == -1 && fork_pid[message.Identifiant] != 0) {
                printf("#SERVEUR : Le client %d vient de se déconnecter", message.Identifiant);
                kill(fork_pid[message.Identifiant], SIGINT);
                clients[message.Identifiant] = 0;
            } else if (nbConnectedClients() < MAX_CLIENT && isConnectedClients(message.Identifiant) != true) {
                clients[message.Identifiant] = message.Pid;
                printf("-----------ACCEPTED-----------\n");

                // CREATION DU PROCESSUS FILS
                // Erreur du fork à gérer (todo)
                // On save le fork id pour le delete correctement plus tard
                fork_pid[message.Identifiant] = fork();

                if (fork_pid[message.Identifiant] == 0) {
                    /* process fils */
                    execl("./chat_child","chat_child", message.Texte, NULL);
                    perror("EXECL : ");
                } else {
                    /* process père */
                }
            } else {
                printf("REFUSED\n");
                kill(message.Pid, SIGUSR1);
            }
        }
    }

    // on détruit le fifo 
    unlink(DEFAULT_PIPE);
    return 0;
}

// Gestion des signaux
void signal_handler(int num_sig) {
    printf("\nJ'ai recu un signal d'interruption : %d\n", num_sig);
    printf("# Je m'éteins !\n" );
    disconnectAllClients();
    unlink(DEFAULT_PIPE);
    exit(1);
}

// Est-ce que le client est connecté ?
int isConnectedClients(int nb) {
    if (clients[nb] != 0)
        return true;
    return false;
}

// On compte le nb de clients connectés
int nbConnectedClients() {
    int nb = 0;
    for (int i = 0; i < 9; ++i)
        if (clients[i] != 0)
            ++nb;
    return nb;
}

//On déconnecte tous les clients (feature utile lorsqu'il y a plusieurs comptes connectés)
void disconnectAllClients() {
    for (int i = 0; i < 9; ++i)
        if (clients[i] != 0)
            kill(clients[i], SIGUSR2);
}
