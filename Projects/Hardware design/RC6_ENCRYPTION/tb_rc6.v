`timescale 1ns / 1ps

module dut_tb();

    reg clk, rst;
    
    // key scheduler wires and regs
    reg start, valid_data;
    wire valid;
    reg [7:0] data;
    wire [1407:0] key;

    // encryption wires and regs
    reg trigger;
    reg [31:0] A_in, B_in, C_in, D_in;
    reg [1407:0] key_reg;
    wire [31:0] A_out, B_out, C_out, D_out;
    wire rc_valid;

    key_gen kg(clk, rst, start, valid_data, data, key, valid);
    rc6 rc(clk, rst, A_in, B_in, C_in, D_in, key_reg, trigger, A_out, B_out, C_out, D_out, rc_valid);
    
    always #5 clk = ~clk;
    
    
    initial begin
        clk = 0;
        $dumpfile("simulacia.vcd");
        $dumpvars(0, dut_tb); // '0' znamená nahraj všetko v tomto module a nižšie

        /* key_gen test */

        // start state reset
        rst = 1;
        start = 0;
        trigger = 0;
        valid_data = 0;
        A_in = 0;
        B_in = 0;
        C_in = 0;
        D_in = 0;

        
        // just to be sure
        repeat (3) @(posedge clk); 
        
        @(negedge clk);
        rst <= 0;

        // module test start
        repeat (3) begin
            @(negedge clk);
            start <= 1;
            data <= 8'd16;
            
            @(negedge clk);
            start <= 0;
            data <= 8'd0;
            valid_data <= 1'b1;

            repeat (16) @(negedge clk);

            wait (valid == 1);
            key_reg <= key;
            $display("%h", key);
            #50;
        end

        /* rc6 test */
        @(negedge clk);
        A_in <= 0;
        B_in <= 0;
        C_in <= 0;
        D_in <= 0;
        
        repeat (3) begin
            @(negedge clk);
            trigger <= 1;

            @(negedge clk);
            trigger <= 0;

            wait (rc_valid == 1);
            $display("%h%h%h%h", A_out, B_out, C_out, D_out);
            #20;
        end
        

        
        // end simulation
        $display("Simulacia dokoncena!\n");
        
        $finish;
    end

    initial #500000 $finish;

endmodule