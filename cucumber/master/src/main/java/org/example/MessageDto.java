package org.example;

import lombok.*;

@Getter
@ToString
@EqualsAndHashCode
@AllArgsConstructor(staticName = "of")
@NoArgsConstructor(access = AccessLevel.PRIVATE)
public class MessageDto {
    private int id;
    private String message;
}
