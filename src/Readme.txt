mictcp-v1:
Toutes les fonctions implémentés marchent trés bien.
---> Compilation : make
---> Test de mictcp avec tsock_texte : 
             -.tsock_texte -p/-s
---> Test de mictcp avec tsock_video :
             -./tsock_video -p/s -t mictcp
--------------------------------------------------------------------------------------------

mictcp-v2 :
Toutes les fonctions implémentés marchent trés bien, le protocole grantie le reception du paquet en le renvoyant plusieurs jusqu'à la bonne reception du paquet de la part du destinataire.
---> Compilation : make
---> Test de mictcp avec tsock_texte : 
             -.tsock_texte -p/-s
---> Test de mictcp avec tsock_video :
             -./tsock_video -p/s -t mictcp


--------------------------------------------------------------------------------------------

mictcp-v3 :
Toutes les fonctions implémentés marchent trés bien.
---> Compilation : make
---> Test de mictcp avec tsock_texte : 
             -.tsock_texte -p/-s
---> Test de mictcp avec tsock_video :
             -./tsock_video -p/s -t mictcp
Nous avons fait plusieurs test avec des pourcentages de pertes toleres et des pourcentages de pertes de simulation differents.
Le mecanisme de reprise des pertes partielles est basé sur l'implementation d'une fenetre glissante.
--------------------------------------------------------------------------------------------


mictcp-v4.1 :
Toutes les fonctions implémentés marchent trés bien.
---> Compilation : make
---> Test de mictcp avec tsock_texte : 
             -.tsock_texte -p/-s
---> Test de mictcp avec tsock_video :
             -./tsock_video -p/s -t mictcp
Nous avons fait plusieurs test avec des pourcentages de pertes toleres et des pourcentages de pertes de simulation differents.
Le mecanisme de reprise des pertes partielles est basé sur l'implementation d'une fenetre glissante.
En plus , une négociation des pourcentages de pertes à adapter est prise en compte, en choisissant le pourcentage le plus petit.