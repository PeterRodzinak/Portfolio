`timescale 1ns/1ps

module tb_rsa ();

    // Nastavenie parametrov podla modulu
    localparam BIT_WIDTH = 128;
    localparam MOD = 128'ha656c5fb97afa0eed886a53380706f2b;
    localparam MU  = 129'h189fd9d74cac687728f0bd7aac5c44139;

    // Signaly pre ovladanie DUT
    reg clk;
    reg rst;
    reg start;
    reg [255:0] op1;
    reg [127:0] expected_res;

    wire [127:0] res;
    wire valid;

    // Prepojovacie vodice medzi Barrettom a nasobickou
    wire [255:0] mult_op0;
    wire [31:0]  mult_op1;
    wire [287:0] mult_res;

    // --- Instancia redukcie ---
    Barrett_reduction #(
        .BIT_WIDTH(BIT_WIDTH)
    ) dut_barrett_reduction (
        .clk(clk),
        .rst(rst),
        .start(start),
        .op1(op1),
        .mu(MU),
        .mod(MOD),
        
        // Komunikacia s nasobickou
        .mult_op0(mult_op0),
        .mult_op1(mult_op1),
        .mult_res(mult_res),
        
        .res(res),
        .valid(valid)
    );

    localparam [31:0] MOD_PRIME = 32'h11b1287d;
    
    reg start_mont;
    reg [127:0] op0_mont;
    reg [127:0] op1_mont;
    reg [127:0] expected_res_mont;

    wire [127:0] res_mont;
    wire valid_mont;

    // Prepojovacie vodice medzi Montgomerym a jeho nasobickou
    wire [255:0] mult_op0_mont;
    wire [31:0]  mult_op1_mont;
    wire [287:0] mult_res_mont;

    Montgomery_reduction #(
        .BIT_WIDTH(BIT_WIDTH),
        .WORD_WIDTH(32)
    ) dut_montgomery (
        .clk(clk),
        .rst(rst),
        .start(start_mont),
        .op0(op0_mont),
        .op1(op1_mont),
        .mod(MOD),
        .mod_prime(MOD_PRIME),
        
        .mult_op0(mult_op0_mont),
        .mult_op1(mult_op1_mont),
        .mult_res(mult_res_mont),
        
        .res(res_mont),
        .valid(valid_mont)
    );

    // --- Instancia nasobicky PRE MONTGOMERYHO ---
    Multiplier dut_multiplier_mont (
        .clk(clk),
        .op0(mult_op0_mont),
        .op1(mult_op1_mont),
        .res(mult_res_mont)
    );

    // --- Instancia nasobicky PRE BARRETTA ---
    Multiplier dut_multiplier (
        .clk(clk),
        .op0(mult_op0),
        .op1(mult_op1),
        .res(mult_res)
    );

    reg rsa_start;
    reg rsa_cipher_flag;
    reg [255:0] rsa_plain_text;
    wire [255:0] rsa_cipher_text;
    wire rsa_valid;

    RSA #(
        .BIT_WIDTH(BIT_WIDTH)
    ) dut_rsa (
        .clk(clk),
        .rst(rst),
        .start(rsa_start),
        .plain_text(rsa_plain_text),
        .cipher_flag(rsa_cipher_flag),
        
        .cipher_text(rsa_cipher_text),
        .valid(rsa_valid)
    );

    // Generovanie hodin (perioda 10ns -> 100MHz)
    always #5 clk = ~clk;

    // Hlavny testovaci proces
    initial begin
        // Nastavenie dumpovania pre waveformy
        $dumpfile("tb_rsa.vcd");
        $dumpvars(0, tb_rsa);

        // Pociatocna inicializacia
        clk = 0;
        rst = 1;
        start = 0;
        op1 = 256'd0;

        // Podrzime reset zopar taktov
        #20;
        @(posedge clk);
        rst = 0;
        #10;

        // ==========================================
        // SPRACOVANIE TESTU BARRETT
        // ==========================================
        $display("\n=== Spustam test Barrett ===");
        
        // Hodime tam nejake obrovske testovacie cislo
        // Mozes sem prekopirovat hodnotu z Python skriptu pre presne porovnanie
        op1 = 256'hDEADBEEFCAFEBABE123456789ABCDEF0DEADBEEFCAFEBABE123456789ABCDEF0;
        
        @(posedge clk);
        start = 1; // Pulz pre zaciatok vypoctu
        
        @(posedge clk);
        start = 0;

        // Cakame, kym modul neohlasi, ze skoncil
        wait(valid == 1'b1);
        @(posedge clk); // Pockame este jeden takt, nech mame vo waveforme peknu hranu
        
        // Vypiseme vysledok do konzoly
        $display("==================================================");
        $display("Test ukonceny v case %0t", $time);
        $display("Vstup op1:      %x", op1);
        $display("Vysledok res:   %x", res);
        $display("==================================================");

        // ==========================================
        // SPRACOVANIE TESTU MONTGOMERY
        // ==========================================
        #20; // Kratka pauza po Barrettovi
        $display("\n=== Spustam test Montgomery ===");
        
        // Pociatocny stav
        start_mont = 0;
        
        // Testovacie 128-bit operandy z Pythonu
        op0_mont = 128'h1234567890abcdef1234567890abcdef;
        op1_mont = 128'hfedcba0987654321fedcba0987654321;
        
        // Očakávaný výsledok operácie: (op0 * op1 * R^-1) mod MOD (kde R = 2^128)
        expected_res_mont = 128'h1b7db41aeb20970c687c87bdf06b07af; 
        
        @(posedge clk);
        start_mont = 1; // Pulz pre zaciatok
        
        @(posedge clk);
        start_mont = 0;

        // Cakame, kym modul neohlasi skoncenie
        wait(valid_mont == 1'b1);
        @(posedge clk); 
        
        // Vyhodnotenie
        $display("Vstup op0_mont: %x", op0_mont);
        $display("Vstup op1_mont: %x", op1_mont);
        $display("Vysledok res:   %x", res_mont);
        
        if (res_mont === expected_res_mont) begin
            $display("MONTGOMERY STATUS: [ PASS ]");
        end else begin
            $display("MONTGOMERY STATUS: [ FAIL ]");
            $display("Ocakavane:         %x", expected_res_mont);
        end
        $display("==================================================");

        // ==========================================
        // SPRACOVANIE TESTU RSA (TOP MODUL)
        // ==========================================
        #100; // Pauza nech sa trochu precistia waveformy
        $display("\n=== Spustam test RSA Top Modulu ===");
        
        // Pripravime vstupy
        rsa_start = 0;
        rsa_cipher_flag = 1'b1;
        
        // Hodime mu nejaku testovaciu 256-bitovu spravu (mozes zmenit za svoju z Pythonu)
        rsa_plain_text = 256'h00112233445566778899AABBCCDDEEFF_FFEEDDCCBBAA99887766554433221100;
        
        // Reset pre istotu
        rst = 1;
        #20;
        @(posedge clk);
        rst = 0;
        #10;
        
        // Odstartujeme vypocet
        @(posedge clk);
        rsa_start = 1;
        
        @(posedge clk);
        rsa_start = 0;
        
        $display("Cakam na ukoncenie vypoctu (toto moze trvat tice taktov!)...");
        
        // Cakame na valid
        wait(rsa_valid == 1'b1);
        @(posedge clk);
        
        $display("==================================================");
        $display("RSA Test ukonceny v case %0t", $time);
        $display("Vstup plain_text: %x", rsa_plain_text);
        $display("Rezimu (cipher_flag): %b", rsa_cipher_flag);
        $display("Vysledok cipher_text: %x", rsa_cipher_text);
        $display("==================================================");
        
        #50;
        $finish; // Ukoncenie simulacie
    end

endmodule