/*
 * Copyright (C) 2017 relayr GmbH
 * Antonio Lignan <antonio.lignan@relayr.io>
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 * THE SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __RELAYR_MQTT_H__
#define __RELAYR_MQTT_H__

#include "../MQTT/MQTT.h"
#include "../SparkJson/SparkJson.h"


/* -----------------------------------------------------------------------------
 * Overridable constants
 * ---------------------------------------------------------------------------*/
#ifndef MQTT_SERVER
#define MQTT_SERVER       "mqtt.relayr.io"
#endif
#ifndef MQTT_PORT
#define MQTT_PORT         1883
#endif
#ifndef MQTT_QOS
#define MQTT_QOS          1
#endif
#ifndef MQTT_BUFFER_SIZE
#define MQTT_BUFFER_SIZE  512
#endif
/* -----------------------------------------------------------------------------
 * Enumerations and structures
 * ---------------------------------------------------------------------------*/
/* Format types to encode the data to be published */
typedef enum {
  JSON_IS_INT = 0,
  JSON_IS_UINT,
  JSON_IS_FLOAT,
  JSON_IS_STRING,
  JSON_IS_BOOL,
  JSON_MAX_TYPES,
} relayr_format_type_t;

/* MQTT broker connection information */
typedef struct mqtt_data {
  const String user;
  const String passwd;
  const String client_id;
} mqtt_data_t;

/* WiFI connection information */
typedef struct wifi_data {
  const IPAddress address;
  const IPAddress netmask;
  const IPAddress gateway;
  const IPAddress dns;
} wifi_data_t;

/* Data structure */
typedef struct relayr_data {
   const String meaning;
   float val;
   const float min_value;
   const float max_value;
   const float def_value;
   const relayr_format_type_t type;
} relayr_data_t;

/* -----------------------------------------------------------------------------
 * API
 * ---------------------------------------------------------------------------*/
/* MQTT connection and subscription */
bool relayr_mqtt_connect(mqtt_data_t *mqtt, void (*cmd)(const char*, void*, uint8_t),
                         void (*cfg)(const char*, void*, uint8_t));

/* Poll the MQTT connection */
bool relayr_mqtt_poll(void);

/* MQTT publish a previously encoded string */
bool relayr_mqtt_publish(mqtt_data_t *mqtt, relayr_data_t *buf, uint8_t num,
                         MQTT::EMQTT_QOS qos);

/* Check the WiFI connection by sending a ping message to the DNS server */
bool relayr_check_wifi_connection(wifi_data_t *buf);

/* Checks if the values stored in .val are valid (boundary check) */
void relayr_check_values(relayr_data_t *buf, uint8_t num);

/* Encodes the readings into the expected JSON format */
void relayr_encode_values(relayr_data_t buf[], uint8_t num);

#endif /* __RELAYR_MQTT_H__ */
