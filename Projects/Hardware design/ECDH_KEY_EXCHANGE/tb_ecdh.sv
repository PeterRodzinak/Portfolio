`timescale 1ns / 1ps

module tb_ecdh();

    reg clk;
    reg rst;
    reg start;
    reg [78:0] p_x, p_y, key;
    reg p_inf;

    wire [78:0] res_x, res_y;
    wire valid, res_inf;

    ECDH_CORE uut (
        .clk        (clk),
        .rst        (rst),
        .start      (start),
        
        .key        (key),
        .p_x        (p_x),
        .p_y        (p_y),
        .p_inf      (p_inf),
        
        .res_x      (res_x),
        .res_y      (res_y),
        .res_inf    (res_inf),
        .valid      (valid)
    );

    always #5 clk = ~clk;

    initial begin
        $dumpfile("waves.vcd");
        $dumpvars(0, tb_ecdh);
        
        // Inicializácia
        clk = 0;
        rst = 1;
        start = 0;
    
        // Uvoľnenie resetu
        #20 rst = 0;
        #10;
        
        key = 79'h43331c80317fa3b1799d;
        p_x = 79'h30cb127b63e42792f10f;
        p_y = 79'h547b2c88266bb04f713b;
        p_inf = 0;
        start = 1;
        #10 start = 0; // start signál držíme 1 takt
        
        wait(valid); // Čakáme kým násobička dokončí prácu
        $display("Res_x: %h\n", res_x);
        $display("Res_y: %h\n", res_y);
        $display("Res_inf: %h\n", res_inf);
        
        #20;
        $finish;
    end

endmodule