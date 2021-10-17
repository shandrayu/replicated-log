package org.example;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import lombok.extern.slf4j.Slf4j;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;


@Slf4j
public class RootHandler implements HttpHandler {
    private final Map<Integer, String> messages;
    private final NotificationService service;

    public RootHandler(String[] secondaries) {
        this.messages = new ConcurrentHashMap<>();
        this.service = new NotificationService(secondaries);
    }

    @Override
    public void handle(HttpExchange exchange) throws IOException {
        switch (Request.fromString(exchange.getRequestMethod())) {
            case GET:
                var msgs = new ArrayList<>(messages.entrySet());
                log.info("Return all msgs {}", msgs);
                String body = JsonUtils.listToJson(msgs);
                exchange.sendResponseHeaders(Response.OK, body.length());
                exchange.getResponseBody().write(body.getBytes());
                exchange.getRequestBody().close();
            case POST:
                var messageDto = JsonUtils.getMassage(exchange.getRequestBody());
                if (messageDto != null) {
                    messages.putIfAbsent(messageDto.getId(), messageDto.getMessage());
                    log.info("Added msg: {}", messageDto);
                    service.notify(messageDto);
                    log.info("Added msg: {}", messageDto);
                    exchange.sendResponseHeaders(Response.OK, 0);
                } else {
                    log.error("Could not deserialize msg");
                    exchange.sendResponseHeaders(Response.ERROR, 0);
                }
                exchange.getRequestBody().close();
            default:
                var errorMsg = String.format("Method unsupported: %s", exchange.getRequestMethod());
                exchange.sendResponseHeaders(Response.ERROR, errorMsg.length());
                exchange.getResponseBody().write(errorMsg.getBytes());
                exchange.getRequestBody().close();
        }
    }
}
