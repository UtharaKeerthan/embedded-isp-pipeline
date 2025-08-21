# Software Requirements Specification — Embedded ISP Pipeline

## ISP Core
| ID           | Requirement                                                  | Priority |
|--------------|--------------------------------------------------------------|----------|
| REQ-ISP-001  | Pipeline shall convert RGGB Bayer input to RGB output        | Mandatory |
| REQ-ISP-002  | No dynamic memory allocation (no malloc / no new)            | Mandatory |
| REQ-ISP-003  | Shall support 1920x1080 resolution                           | Required  |
| REQ-ISP-004  | Per-frame processing latency shall not exceed 33ms           | Required  |

## White Balance
| REQ-WB-001   | Shall apply independent gain per R, G, B channel             | Required  |
| REQ-WB-002   | Gray world algorithm shall be the default mode               | Required  |

## Gamma
| REQ-GAMMA-001 | Shall use LUT-based gamma (no floating-point at runtime)    | Mandatory |
| REQ-GAMMA-002 | Gamma value configurable in range [1.0 - 3.0]               | Required  |

## Edge Detection
| REQ-EDGE-001  | Sobel shall output gradient magnitude and direction          | Required  |
| REQ-EDGE-002  | Canny shall implement non-maximum suppression                | Required  |
| REQ-EDGE-003  | Canny hysteresis thresholds shall be configurable            | Required  |

## Color
| REQ-COLOR-001 | CCM shall apply 3x3 matrix in Q4.12 fixed-point              | Required  |
| REQ-COLOR-002 | RGB to YUV conversion shall use BT.709 coefficients          | Required  |

## Histogram & Analysis
| REQ-HIST-001  | Per-channel histogram over 256 bins                         | Required  |
| REQ-HIST-002  | CLAHE tile size shall be configurable                        | Optional  |
| REQ-AE-001    | Auto-exposure convergence within 10 frames                   | Required  |
| REQ-DPC-001   | Dead pixels replaced using 4-neighbour median                | Required  |
| REQ-RESIZE-001| Bilinear interpolation shall be used for scaling             | Required  |

## Memory
| REQ-MEM-001   | Stack usage per function shall not exceed 4096 bytes         | Mandatory |
