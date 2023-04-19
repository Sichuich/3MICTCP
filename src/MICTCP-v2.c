#include <api/mictcp_core.h>
#include <mictcp.h>

#define max_envoi 500

/*declaration des variables*/
mic_tcp_sock mysock;
mc_tcp_sock_addr @sock_dest;
int PE = 0;
int PA = 0;

/*creation d'un socket entre l'application et mic-tcp
*rettourne le descriptzur du socket ou  -1 en cas d'erreur*/
int mic_tcp_socket(start_mode sm){
     printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");

     if(initialize_components(sm)!=-1){
        mysock.state = IDLE ;
        mysock.fd = 0 ;
        return mysock.fd ; /*descripteur du socket */
     }
     else{
        return -1 ; /*echec lors de la creation du socket*/
     }
}

/*Attribution d'une @ au socket*/
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr){
         printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
         if (mysock.fd==socket){
            mysock.addr=addr;
            return 0; /*attribution reussie*/
         }
         else{
            return -1; /*attribution d'@ echoue*/
        }
}

/*Mettre le socket en etat  d'acceptation de connexion*/
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr){
        printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
        if(mysock.fd == socket){
            mysock.state = Connected ;
            return 0; /*socket pret a accepter les connexions*/
        }
        else{
            return -1; /*echec d'operation*/
        }
}

/*confiramation de l'etablissement de connexion*/
int mic_tcp_connect(int socket, mic_tcp_sock_addr addr){
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    if(mysock.fd == socket){
        mysock.state = Connected;
        addr_sock_dest = addr;
        return 0; /*etablissement de connexion reussie*/
    }
    else{
        return -1; /*echec d'eatblissement de connexion*/
    }
}

/*envoi de PDU*/
int mic_tcp_send(int mic_sock, char* mesg, int mesg_size){
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    int size_send;
    int nb_envoi = 0;
    mic_tcp_pdu pdu_send;
    unsigned long timeout = 500;
    int ack_recv = 0;

    if((mysock.state == Connected) && (mysock.fd == mic_sock) ){
        /*constrution du PDU a emettre */
        /*header*/
        pdu_send.header.source_port = mysock.addr.port;
        pdu_send.header.dest_port = addr_sock_dest.port;
        pdu_send.header.ack = 0;
        pdu_send.header.syn = 0;
        pdu_send.header.fin = 0;
        pdu_send.header.seq_num = PE;

        /*DU*/
        pdu_send.payload.data = mesg;
        pdu_send.payload.size = mesg_size;

        PE =(PE + 1) % 2;

        /*envoi du pdu */
        size_send = IP_send(pdu_send, @sock_dest);
        nb_envoi ++ ;

        /*attente d'un ack */
        mysock.state =  WAIT_FOR_ACK ;

        /*construction d'ack */
        ack.payload.size = 2*sizeof(short)2*sizeof(int)+3*sizeof(char);
        ack.payload.data = malloc(ack.payload.size);

        while(!ack_recv){
            if((ack.header.ack == 1) && (ack.header.ack_num == PE) && (IP_recv(&(ack),&@sock_dest, timeout >=0))){
                /*reception du pdu avec PE == num*/
                ack_recv = 1;
            }
            else{
                /*expiration du timer (retransmission)*/
                if(nb_envoi < Max_ENVOI){
                    size_send = IP_send(pdu_send, @sock_dest);
                    nb_envoi++;
                }
            }
        }
    }
    else{
        return -1 ; /*erreur n_socket ou prob de connexion*/
    }

    mysock.state = Connected;
    return size_send;
}

/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
 * NB : cette fonction fait appel à la fonction app_buffer_get()
 */
int mic_tcp_recv (int socket, char* mesg, int max_mesg_size) //prend dans le buffer pour le donner à l'appli
{
  int deliver_size = -1;
  mic_tcp_payload Payload;
  
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");

  Payload.data = mesg;
  Payload.size = max_mesg_size;
    
  if ((socket == sock.fd) && (sock.state == CONNECTED)) {
    //WAIT for un PDU
    sock.state = WAIT_PDU ;    

    //récupérer la donnée applicative contenue dans le buffer 
    deliver_size = app_buffer_get(Payload);

    //revient state
    sock.state = CONNECTED ;
  }
  return deliver_size;  
}

/*
 * Permet de réclamer la destruction d’un socket.
 * Engendre la fermeture de la connexion suivant le modèle de TCP.
 * Retourne 0 si tout se passe bien et -1 en cas d'erreur
 */
int mic_tcp_close (int socket)
{
    printf("[MIC-TCP] Appel de la fonction :  "); printf(__FUNCTION__); printf("\n");
    return -1;
}

/*
 * Traitement d’un PDU MIC-TCP reçu (mise à jour des numéros de séquence
 * et d'acquittement, etc.) puis insère les données utiles du PDU dans
 * le buffer de réception du socket. Cette fonction utilise la fonction
 * app_buffer_put().
 */
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_sock_addr addr) 
{
  mic_tcp_pdu ack;

  printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");

  if (pdu.header.seq_num == PA) { // n_seq == PA -->acceptation de DT
    
    /* Ajout de la charge utile du PDU recu dans le buffer de reception */
    app_buffer_put(pdu.payload);
    PA = (PA +1) % 2;
  }
  // sinon, n°seq != PA, rejet de la DT => PA reste le même

    /* Construction d'un ACK */
    // Header
    ack.header.source_port = mysock.addr.port;
    ack.header.dest_port = addr.port;
    ack.header.ack = 1;
    ack.header.syn = 0;
    ack.header.fin = 0;
    ack.header.ack_num = PA;
    ack.payload.size = 0; // pas de DU

    /* Envoi de l'ACK */
    IP_send(ack, addr);
}
