// Voltage Differential Sense Amplifier
module voltage_diff_sense_amp (
    input wire clk,        // Clock for synchronization
    input wire SA_signal,  // Input signal to be compared
    input wire SA_ref,     // Reference voltage
    output reg SA_out      // Output: 1 if SA_signal > SA_ref, else 0
);
    always @(posedge clk) begin
        if (SA_signal > SA_ref)
            SA_out <= 1;  // High if signal > reference
        else
            SA_out <= 0;  // Low otherwise
    end
endmodule

// D Flip-Flop with Asynchronous Reset
module DFFR_X1 (
    input wire D,
    input wire CK,
    output reg Q,
    output reg QN
);
    always @(posedge CK) begin
        Q <= D;
        QN <= ~D;
    end
endmodule

// Top-level Module
module modified_circuit_with_SA (
    input wire clkA,        // Clock for Flip-Flop A
    input wire clkB,        // Clock for Flip-Flop B
    input wire SA_signal,   // Sense Amplifier Input Signal
    input wire SA_ref,      // Sense Amplifier Reference Signal
    output wire M          // Output M
);

    wire SA_out;
    wire Q_A, Q_B, Q_A_bar;

    // Instantiate Sense Amplifier
    voltage_diff_sense_amp SA (
        .clk(clkA), 
        .SA_signal(SA_signal), 
        .SA_ref(SA_ref), 
        .SA_out(SA_out)
    );

    // Instantiate Flip-Flop A
    DFFR_X1 FF_A (
        .D(SA_out),
        .CK(clkA),
        .Q(Q_A),
        .QN(Q_A_bar)  // Using Q' instead of an inverter
    );

    // Instantiate Flip-Flop B
    DFFR_X1 FF_B (
        .D(SA_out),
        .CK(clkB),
        .Q(Q_B),
        .QN()
    );

    // OR Gate for final output
    assign M = Q_A_bar | Q_B;

endmodule
