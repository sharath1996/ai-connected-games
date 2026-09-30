|Description|Serial Comamnd | Arguments | full serial command|
|------------|--------------|-----------|--------------------|
|LED Array Static Display (Individual)| LEDSTATICIND |[LED_Index , R , G , B]|LEDSTATICIND 2 255 255 255|
|LED Array static Display (Group) | LEDSTATICGRP | [R, G, B]|LEDSTATICGRP 255 0 0|
|LED Array Animation (Group) | LEDANIMGRP | R, G, B, Start brightness, stop brightness, duration| LEDANIMGRP 255 1 0 0 255 10|
|Buzzer notifiy| BUZZER_NTF | On / Off (1,0) | BUZZER_NTF 1, BUZZER_NTF 0|
|OLED Display | OLED_DISP | character | OLED_DISP ♥|

And if the switch is pressed, it should be defaulting to off or normal position