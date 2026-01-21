# Spécification d'Interface Linky -> Knob (MQTT)

Pour que l'afficheur "Knob" fonctionne, le module Linky doit publier les données sur un topic MQTT.

## Configuration MQTT
- **Broker** : Adresse IP du broker (ex: `192.168.1.x` ou Mosquitto HA).
- **Topic** : `home/linky/status` (Exemple, à convenir).
- **Fréquence** : Idéalement toutes les 5 à 10 secondes (ou à chaque changement significatif).

## Format des Données (Payload JSON)
Le payload doit être un objet JSON plat.

```json
{
  "papp": 450,           // Puissance Apparente (VA) - Entier
  "base": 12345678,      // Index Base (Wh) - Optionnel si HP/HC non utilisé
  "hp": 9876543,         // Index Heures Pleines (Wh) - Entier
  "hc": 1234567,         // Index Heures Creuses (Wh) - Entier
  "iinst": 2,            // Intensité Instantanée (A) - Entier
  "voltage": 232         // Tension (V) - Entier
}
```

### Champs Obligatoires
*   `papp` (pour la jauge principale).
*   Au moins un des index (`base` OU `hp`+`hc`).

### Exemple de message minimal
```json
{ "papp": 1200, "base": 50000 }
```
