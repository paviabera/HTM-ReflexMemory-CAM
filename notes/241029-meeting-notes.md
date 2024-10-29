# Data Input Flow

```mermaid

flowchart TD;
    A[Data] --> C;
    C[Encoder] --> D[Reflex Memory];
    C --> E;
    E[Spatial Pooler] --> F[Sequence Memory];

```

# Reflex Memory

| SDR1    | SDR2 | Occurences | Last Accessed |
| -------- | ------- | ------- | ------- |
| dense_sdr_1 | dense_sdr_2 | count_1 | access_time_1 |
| dense_sdr_1 | dense_sdr_3 | count_2 | access_time_2 |
| dense_sdr_1 | dense_sdr_4 | count_2 | access_time_3 |

### An advantage of dense SDRs stored in reflex memory is that the data is still decodable. Spatial Pooler is a one-way function, so they are not decodable after this step. 

### Additionally, storing dense SDRs eliminates needing to wait for Spatial Pooler finish learning. If Spacial Pooler isn't done learning, the same input could get different sparse outputs. 


# Prediction Comparison Flow

```mermaid

flowchart TD;
    A[Sequence Memory] --> B[Prediction];
	C[Dense SDR] --> D;
	D[Reflex Memory] --> E[Spatial Pooler];
	E --> F[Sparse SDR];
	F --> G[COMPARE];
	B --> G;
	G --> H[Retrieve Dense SDR <br/> and Decode];

```

# Prediction Scenarios

| SM | RM | Result |
| -------- | ------- | ------- |
| 0 | 0 | Penalize RM |
| 0 | 1 | Update SM permanences |
| 1 | 0 | High Penalty RM. Add SM to RM. |
| 1 | 1 | Update SM permanences |


# Decoding Flow

```mermaid

flowchart TD;
    A[Predicted Sparse SDR] --> F;

    C[Dense Reflex Memory Table] --> D[Spatial Pooler per row];
    D --> E[Sparse Reflex Memory Table];
    E --> F[Compare <br/> Match];
    F --> G[Decode Dense SDR];


```
