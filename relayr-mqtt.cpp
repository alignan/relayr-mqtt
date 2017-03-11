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

#include "relayr-mqtt.h"
#include "../MQTT/MQTT.h"
#include "../SparkJson/SparkJson.h"
#include "Particle.h"

#ifndef ENABLE_DIAGNOSTIC
#define ENABLE_DIAGNOSTIC   1
#endif

#if ENABLE_DIAGNOSTIC
float counter = 0;
#endif

/* Buffer used to store the content of the publication */
char message_buff[MQTT_BUFFER_SIZE];

bool relayr_check_wifi_connection(wifi_data_t *buf) {
  int ping;
  ping = WiFi.ping(buf->dns, 3);
  if (ping > 0) return TRUE;
  Serial.println("[RELAYR] Connection not ready");
  return FALSE;
}

/* Connects to the MQTT broker and subscribe to default topics */
bool relayr_mqtt_connect(MQTT *client, mqtt_data_t *mqtt) {
  String topic;
  Serial.println("[RELAYR] Connecting to mqtt server...");

  if(client->connect(mqtt->client_id, mqtt->user, mqtt->passwd)) {
    topic = "/v1/" + String(mqtt->user) + "/config";
    Serial.println(topic);
    client->subscribe(topic, (MQTT::EMQTT_QOS)MQTT_QOS);

    topic.replace("config", "cmd");
    Serial.println(topic);
    client->subscribe(topic, (MQTT::EMQTT_QOS)MQTT_QOS);

    return TRUE;
  }
  Serial.println("[RELAYR] Connection failed, check your credentials or wifi, restarting");
  return FALSE;
}

/* Functions that checks boundaries */
void relayr_check_values(relayr_data_t buf[], uint8_t num) {
  for(uint8_t i=0; i < num; i++) {
    if((buf[i].val < buf[i].min_value) || (buf[i].val > buf[i].max_value)) {
      buf[i].val = buf[i].def_value;
    }
  }
}

/* Serialize the readings into the expected relayr's JSON template */
static void relayr_json_encode(JsonObject& obj, void *value, const char *meaning,
                               const char *path, uint8_t type) {
  if((meaning == NULL) || (value == NULL) || (value == NULL) ||
    (type > JSON_MAX_TYPES)) {
    Serial.println("[RELAYR] Invalid JSON encoding arguments");
    return;
  }

  obj["meaning"] = meaning;

  switch(type) {
    case JSON_IS_FLOAT:
    {
      float *reading = (float *)value;
      obj["value"] = *reading;
    }
    break;
    case JSON_IS_STRING:
    {
      char *reading = (char *)value;
      obj["value"] = *reading;
    }
    break;
    case JSON_IS_UINT:
    {
      float *reading = (float *)value;
      obj["value"] = (uint16_t)*reading;
    }
    break;
    case JSON_IS_INT:
    {
      float *reading = (float *)value;
      obj["value"] = (int16_t)*reading;
    }
    break;
    default:
      break;
  }
}

/* Creates the expected topic string and publishes */
bool relayr_mqtt_publish(MQTT *client, mqtt_data_t *mqtt, relayr_data_t *buf,
                         uint8_t num, MQTT::EMQTT_QOS qos) {
  String topic = "/v1/" + String(mqtt->user) + "/data";
  /* FIXME: Hard-coded to fixed use case, change into a list */
  StaticJsonBuffer<MQTT_BUFFER_SIZE> pubJsonBuffer;
  char message_buff[MQTT_BUFFER_SIZE];
  JsonArray& pubJson = pubJsonBuffer.createArray();
  JsonObject& temp = pubJson.createNestedObject();
  JsonObject& humd = pubJson.createNestedObject();
  JsonObject& batt = pubJson.createNestedObject();

/* Fixed types used for debugging */
/* FIXME: diagnostic variables should be maintained locally not externally */
#if ENABLE_DIAGNOSTIC
  counter++;
  float rssi_val = WiFi.RSSI();
  JsonObject& cont = pubJson.createNestedObject();
  JsonObject& rssi = pubJson.createNestedObject();
#endif /* ENABLE_DIAGNOSTIC */

  /* FIXME: Hard-coded to fixed use case, change into a list */
  relayr_json_encode(temp, &buf[0].val,
                     buf[0].meaning, "", buf[0].type);
  relayr_json_encode(humd, &buf[1].val,
                     buf[1].meaning, "", buf[1].type);
  relayr_json_encode(batt, &buf[2].val,
                     buf[2].meaning, "", buf[2].type);
#if ENABLE_DIAGNOSTIC
  relayr_json_encode(cont, &counter, "counter", "", JSON_IS_UINT);
  relayr_json_encode(rssi, &rssi_val, "rssi", "", JSON_IS_INT);
#endif /* ENABLE_DIAGNOSTIC */

  pubJson.printTo(message_buff, sizeof(message_buff));
  Serial.printf("[RELAYR] Publishing %s\n", message_buff);
  return client->publish(topic, message_buff, qos);
}
