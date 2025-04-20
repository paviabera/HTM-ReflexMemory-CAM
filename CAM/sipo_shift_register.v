module sipo_shift_register (
    input wire clk,       // Clock signal
    input wire rst,       // Reset (active high)
    input wire serial_in, // Serial data input
    output reg [7:0] parallel_out // Parallel output
);

    always @(posedge clk or posedge rst) begin
        if (rst)
            parallel_out <= 8'b0; // Reset output to 0
        else
            parallel_out <= {parallel_out[6:0], serial_in}; // Shift operation
    end

endmodule
