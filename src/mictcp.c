#include <mictcp.h>
#include <api/mictcp_core.h>

mic_tcp_sock sock;
mic_tcp_sock_addr sock_addr;

/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm)
{
   int result = -1;
   int timer = 1000 ;
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");   
   result = initialize_components(sm); /* Appel obligatoire */
   set_loss_rate(500);

   return result;
}

/*
 * Permet d’attribuer une adresse à un socket.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr)
{
  int result = -1;
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   if (socket == sock.fd) {
     sock.addr = addr ;
     result = 0 ;
   }   
   return result;
}

/*
 * Met le socket en état d'acceptation de connexions
 * Retourne 0 si succès, -1 si erreur
 */
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr)
{
   int result = -1;
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");  
  if (socket == sock.fd) {
    sock.state = CONNECTED;
    result = 0;
  }  
  return result;
}

/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */
int mic_tcp_connect(int socket, mic_tcp_sock_addr addr)
{
   int result = -1;
   
  printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
  
  return result;
}

/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */
int mic_tcp_send (int mic_sock, char* mesg, int mesg_size)
{
  int sentsize = -1 ;
  mic_tcp_pdu PDU;
  
   printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
  
  //géneration PDU-DT contenant
  if ((mic_sock == sock.fd) && (sock.state == CONNECTED)) {   
    PDU.header.source_port = sock.addr.port;
    PDU.header.dest_port = sock_addr.port;
    PDU.header.syn = 0;
    PDU.header.ack = 0;
    PDU.header.fin = 0;

    PDU.payload.data = mesg;
    PDU.payload.size = mesg_size;

    //send PDU
    sentsize = IP_send(PDU, sock_addr);
  }

  return sentsize;

}

/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
 * NB : cette fonction fait appel à la fonction app_buffer_get()
 */
int mic_tcp_recv (int socket, char* mesg, int max_mesg_size)
{
  int deliver_size = -1;
  mic_tcp_payload Payload;
  
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");

  Payload.data = mesg;
  Payload.size = max_mesg_size;
    
  if (socket == sock.fd) and (sock.state == CONNECTED) {
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
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");

    app_buffer_put(pdu.payload);

    sock.state = CONNECTED ;
}


