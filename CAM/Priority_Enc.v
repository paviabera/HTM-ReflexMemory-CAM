module priority_encoder_128bit (
    input  wire [127:0] in,
    output reg  [6:0]   pos,    // 7-bit position output (0–127)
    output reg          valid   // '1' if any bit is high
);
    integer i;
    always @(*) begin
        valid = 1'b0;
        pos   = 7'd0;
        for (i = 127; i >= 0; i = i - 1) begin
            if (in[i]) begin
                pos   = i[6:0];
                valid = 1'b1;
                disable for; // Break out once highest priority '1' is found
            end
        end
    end
endmodule
