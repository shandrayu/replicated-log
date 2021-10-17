package org.example;

import com.fasterxml.jackson.databind.ObjectMapper;
import lombok.experimental.UtilityClass;

import java.io.InputStream;
import java.util.Collection;
import java.util.List;
import java.util.Optional;

@UtilityClass
public class JsonUtils {
    private static final ObjectMapper OBJECT_MAPPER = new ObjectMapper();

    public static MessageDto getMassage(InputStream requestBody){
        try {
            return OBJECT_MAPPER.readValue(requestBody, MessageDto.class);
        } catch (Exception e) {
            //ignore
        }
        return null;
    }

    public static String listToJson(Collection<?> list) {
        return Optional.ofNullable(valueToJson(list)).orElse(List.of().toString());
    }

    public static String valueToJson(Object value) {
        try {
            return OBJECT_MAPPER.writeValueAsString(value);
        } catch (Exception e) {
            //ignore
        }
        return null;
    }
}
