module and_tree_128 (
    input [127:0] in,
    output out
);
    wire [63:0] stage1;
    wire [31:0] stage2;
    wire [15:0] stage3;
    wire [7:0]  stage4;
    wire [3:0]  stage5;
    wire [1:0]  stage6;

    genvar i;
    
    // Stage 1: Reduce 128 inputs to 64
    generate
        for (i = 0; i < 64; i = i + 1) begin
            assign stage1[i] = in[2*i] & in[2*i+1];
        end
    endgenerate

    // Stage 2: Reduce 64 to 32
    generate
        for (i = 0; i < 32; i = i + 1) begin
            assign stage2[i] = stage1[2*i] & stage1[2*i+1];
        end
    endgenerate

    // Stage 3: Reduce 32 to 16
    generate
        for (i = 0; i < 16; i = i + 1) begin
            assign stage3[i] = stage2[2*i] & stage2[2*i+1];
        end
    endgenerate

    // Stage 4: Reduce 16 to 8
    generate
        for (i = 0; i < 8; i = i + 1) begin
            assign stage4[i] = stage3[2*i] & stage3[2*i+1];
        end
    endgenerate

    // Stage 5: Reduce 8 to 4
    generate
        for (i = 0; i < 4; i = i + 1) begin
            assign stage5[i] = stage4[2*i] & stage4[2*i+1];
        end
    endgenerate

    // Stage 6: Reduce 4 to 2
    generate
        for (i = 0; i < 2; i = i + 1) begin
            assign stage6[i] = stage5[2*i] & stage5[2*i+1];
        end
    endgenerate

    // Final Stage: Reduce 2 to 1
    assign out = stage6[0] & stage6[1];

endmodule
