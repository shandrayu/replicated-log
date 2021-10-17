package org.example;

public enum Request {
    GET,
    POST,
    UNSUPPORTED;

    public static Request fromString(String name) {
        for (Request request : Request.values()) {
            if (request.name().equals(name)) {
                return request;
            }
        }
        return UNSUPPORTED;
    }
}
