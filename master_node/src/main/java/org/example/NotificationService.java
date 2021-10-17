package org.example;

import lombok.extern.slf4j.Slf4j;

import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.stream.Collectors;
import java.util.stream.Stream;

@Slf4j
public class NotificationService {
    private final Set<URI> secondaryUris;
    private final ExecutorService executorService;

    public NotificationService(String[] secondaryUris) {
        this.secondaryUris = Stream.of(secondaryUris)
                .map(UrlUtils::getUri)
                .filter(Objects::nonNull)
                .collect(Collectors.toSet());
        this.executorService = Executors.newFixedThreadPool(this.secondaryUris.size() + 1);
    }


    public void notify(MessageDto dto) {
        String body = JsonUtils.valueToJson(dto);
        var futureList = secondaryUris.stream()
                .map(uri -> sendToSecondary(uri, body))
                .collect(Collectors.toList());
        futureList.forEach(
                res -> {
                    try {
                        res.get();
                    } catch (Exception e) {
                        log.error("Could not get response from secondary node: {}", e.getMessage());
                    }
                }
        );

    }

    private CompletableFuture<HttpResponse<String>> sendToSecondary(URI uri, String body){
        log.info("Send message {} to secondary node: {}", body, uri.toString());
        var request = HttpRequest.newBuilder()
                .uri(uri)
                .POST(HttpRequest.BodyPublishers.ofString(body))
                .build();
        return HttpClient.newBuilder()
                .executor(executorService)
                .build()
                .sendAsync(request, HttpResponse.BodyHandlers.ofString());
    }
}
