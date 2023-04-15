#include <mictcp.h>
#include <api/mictcp_core.h>

/*declaration des variables*/
mic_tcp_sock mysock;
mic_tcp_sock_addr addr_sock_dest;

/*creation d'un socket entre l'app et MIC-TCP*/
int mic_tcp_socket(start_mode sm)
{
    printf("[MIC-TCP] Appel de la fonction:"); printf(__FUNCTION__); printf("\n");
     
    if(initialize_components(sm)!=-1){
        mysock.fd =0;
        mysock.state = IDLE;
        return mysock.fd
    }
    else{
        return -1 ;  /*erreur lors de la creation du socket*/
    }
}

/*attribution d'une adresse à un socket*/
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr){
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");

    if(mysock.fd==socket){
        memcpy((char*)&mysock.addr, (char*)&addr, sizeof(mic_tcp_sock_addr));
        return 0;
    }
    else{
        return -1 ; /*erreur lors de l'attribution d'@ au socket*/
    }
}

/*Met le socket en etat d'acceptation de connexions*/
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr){
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");

    if(mysock.fd == socket){
        mysock.state = CONNECTED;
        return 0;
    }
    else{
        return -1;
    }
}

 /*Reclamation de l'etablissement d'une connexion*/
int mic_tcp_connect (int socket, mic_tcp_sock_addr addr) {
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");

    if(mysock.fd == socket){
        mysock.state = CONNECTED;
        addr_sock_dest = addr;
        return 0;
    }
    else{
        return -1;
    }
}

/*Reclamation de l'envoi d'une donnée applicative*/
int mic_tcp_send(int mic_sock,char*mesg, int max_mesg_size){
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");

    
    mic_tcp_pdu pdu_emis;

    if(mysock.fd == mic_sock){
        if(mysock.state == CONNECTED){
          /*Construction du PDU MIC-TCP a emettre :*/
          
          /*--->Header*/
          pdu_emis.header.source_port = mysock.addr.port;
          pdu_emis.header.dest_port =addr_sock_dest.port;
          pdu_emis.header.syn = 0 ;
          pdu_emis.header.ack = 0 ;
          pdu_emis.header.fin = 0 ;

          /*--->Donnee utile du PDU MIC-TCP*/
          pdu_emis.payload.data = mesg;
          pdu_emis.payload.size = mesg_size;

          /*Envoi du PDU MIC-TCP*/
          int size_emis = IP_send(pdu_emis, addr_sock_dest);
        }
        return size_emis ;
    }
}

/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
 * NB : cette fonction fait appel à la fonction app_buffer_get() */

int mic_tcp_recv (int socket, char* mesg, int max_mesg_size) {
	printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");

    mic_tcp_payload pdu;
    pdu.data = mesg;
    pdu.size = max_mesg_size;

    if(mysock.fd == socket){
        if(mysock.state == CONNECTED){
            /*Attente du PDU MIC-TCP*/
            mysock.state = WAIT_FOR_PDU;

            /*Recuperation du PDU MIC-TCP dans le buffer de reception*/
            int np_octets_lus = app_buffer_get(pdu);
            return np_octets_lus ;
        }
        else{
            return -1 ;      /*erreur lors de la recuperation du PDU MIC-TCP*/
        }
    }
}
    /*Traitement du PDU MIC-TCP recu
    Insertion de la donnee utile de PDU MIC-TCP*/
    void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_sock_addr addr) {
        printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
        app_buffer_put(pdu.payload);
    }
/*Fermeture de la connexion - Destruction du socket*/
int mic_tcp_close (int socket) {
	printf("[MIC-TCP] Appel de la fonction :  "); printf(__FUNCTION__); printf("\n");
	return -1; 
}









