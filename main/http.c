#include "http.h"
#include "esp_http_server.h"

static esp_err_t http_get_handler(httpd_req_t *req) {
    const char *html_response = "<!DOCTYPE html>"
        "<html lang=\"en\">"
        "<head>"
        "    <meta charset=\"UTF-8\">"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
        "    <title>Baby Monitor</title>"
        "    <style>"
        "        body {"
        "            font-family: Arial, sans-serif;"
        "            margin: 20px;"
        "        }"
        "        h1 {"
        "            color: #333;"
        "        }"
        "        .status {"
        "            margin-top: 10px;"
        "        }"
        "        .refresh-button {"
        "            margin-top: 20px;"
        "            padding: 10px 20px;"
        "            background-color: #4CAF50;"
        "            color: white;"
        "            border: none;"
        "            border-radius: 5px;"
        "            cursor: pointer;"
        "        }"
        "        .refresh-button:hover {"
        "            background-color: #45a049;"
        "        }"
        "    </style>"
        "</head>"
        "<body>"
        "    <h1>Baby Monitor Status</h1>"
        "    <div class=\"status\">"
        "        <p>Temperature: <span id=\"temperature\">%d</span> &#8451;</p>"
        "        <p>Humidity: <span id=\"humidity\">%d</span> %%</p>"
        "        <p>Noise Level / Threshold: <span id=\"noise\">%d</span> / <span id=\"threshold\">%d</span></p>"
        "    </div>"
        "    <button class=\"refresh-button\" onclick=\"fetchData()\">Refresh</button>"
        "    <script>"
        "        function fetchData() {"
        "            fetch('/status')"
        "                .then(response => {"
        "                    if (!response.ok) {"
        "                        throw new Error(`HTTP error! status: ${response.status}`);"
        "                    }"
        "                    return response.json();"
        "                })"
        "                .then(data => {"
        "                    document.getElementById('temperature').textContent = data.temperature;"
        "                    document.getElementById('humidity').textContent = data.humidity;"
        "                    document.getElementById('noise').textContent = data.noise_level;"
        "                    document.getElementById('threshold').textContent = data.threshold_noise_level;"
        "                })"
        "                .catch(error => {"
        "                    console.error('Error fetching data:', error);"
        "                });"
        "        }"
        "    </script>"
        "</body>"
        "</html>";
    
    char response[2048];
    int len = snprintf(response, sizeof(response), html_response, 
                       temperature, 
                       humidity, 
                       noise_level, 
                       threshold_noise_level);

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, response, len);
    return ESP_OK;
}

static esp_err_t status_get_handler(httpd_req_t *req) {
    char json_response[128];
    int len = snprintf(json_response, sizeof(json_response),
                       "{\"temperature\": %d, \"humidity\": %d, \"noise_level\": %d, \"threshold_noise_level\": %d}",
                       temperature,
                       humidity,
                       noise_level,
                       threshold_noise_level);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_response, len);
    return ESP_OK;
}

void start_http_server(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t uri_get = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = http_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &uri_get);
        
        httpd_uri_t uri_status = {
            .uri       = "/status",
            .method    = HTTP_GET,
            .handler   = status_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &uri_status);
    }
}