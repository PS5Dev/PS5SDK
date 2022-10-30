# Example: klog_server

Contains code to fork and run a continuous klog server over TCP. Runs on port 9081 by default.

## Notes
This payload is firmware specific as it uses `fork()`, which cannot be resolved normally with `dlsym()`. `OFFSET_FORK_FROM_READ` must be added for a particular firmware for it to work. Currently supported on:
- 4.03

## Expected result
Klog client
```
<118>[SceShellCore] CPU Utilization: 0.06 0.00 0.02 0.00 0.00 0.00 0.00 0.00 0.00 0.18 0.00 0.08 0.00 0.09 0.28 0.99
<118>[SceShellCore] CPU Top: timestamp 8992 to 8995 (3.0 sec)
<118>[SceShellCore] 99% 1023 SceIdleCpu12
<118>[SceShellCore] 99% 1023 SceIdl
eCpu10
<118>[SceShellCore] 99% 1023 SceIdleCpu8
<118>[SceShellCore] 99% 1023 SceIdleCpu7
<118>[SceShellCore] 99% 1023 SceIdleCpu6
<118>[SceShellCore] 99% 1023 SceIdleCpu5
<118>[SceShellCore] 99% 1023 SceIdleCpu4
<118>[SceShellCore] 99% 1023 SceIdleCpu3
...
```

## Authors
- Specter
