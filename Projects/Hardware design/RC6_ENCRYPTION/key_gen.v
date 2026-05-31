// module of key scheduler, using official RC6 key schedule procedure as a reference
module key_gen (
    input clk,
    input rst,

    input wire start,
    input wire valid_data,
    input wire [7:0] data,

    output wire [1407:0] key,
    output reg valid
);

    // state of overall key scheduling
    localparam [2:0]
        IDLE         = 3'd0, // module is waiting for start signal and length
        DATA_LOADING = 3'd1, // loads at most 132 bytes of data, remaining fields are filled with 0
        KEY_PROCESS  = 3'd2, // processing the key according to the standard procedure
        KEY_YIELD    = 3'd3, // puts the key data and valid signal on the output, resets the state machine, sends it to IDLE state
        PREPARE      = 3'd4; // state for preparation of all variables, securing safe execution of new key schedule

    reg [2:0] state; // state machine for the whole key scheduling process

    reg [7:0]  len_index;      // index for key array, should only be used on values less than 5'd44
    reg [31:0] key_arr[0:43];  // array of key words
    reg [31:0] S[0:43];        // array S from rc6 key schedule specification
    reg [7:0]  len_reg;        // length of key obtained from input in bytes, is later used in words
    reg        pipeline_ctr;   // reg for pipelining key processing
    reg [7:0]  round_counter;  // reg for counting rounds of key scheduling

    // key schedule helper regs
    reg [31:0] A, B, i, j, rotSAB, rotLAB, ABrot;
    wire [31:0] rot1, rot2;

    // barrel shift modules, using helper regs for faster round
    rol32_barrel barrel1(
        .x(rotSAB),
        .s(5'd3),
        .y(rot1)
    );

    rol32_barrel barrel2(
        .x(rotLAB),
        .s(ABrot[4:0]),
        .y(rot2)
    );

    integer gen_i;

    // key output
    assign key[31: 0] = S[0];
    assign key[63: 32] = S[1];
    assign key[95: 64] = S[2];
    assign key[127: 96] = S[3];
    assign key[159: 128] = S[4];
    assign key[191: 160] = S[5];
    assign key[223: 192] = S[6];
    assign key[255: 224] = S[7];
    assign key[287: 256] = S[8];
    assign key[319: 288] = S[9];
    assign key[351: 320] = S[10];
    assign key[383: 352] = S[11];
    assign key[415: 384] = S[12];
    assign key[447: 416] = S[13];
    assign key[479: 448] = S[14];
    assign key[511: 480] = S[15];
    assign key[543: 512] = S[16];
    assign key[575: 544] = S[17];
    assign key[607: 576] = S[18];
    assign key[639: 608] = S[19];
    assign key[671: 640] = S[20];
    assign key[703: 672] = S[21];
    assign key[735: 704] = S[22];
    assign key[767: 736] = S[23];
    assign key[799: 768] = S[24];
    assign key[831: 800] = S[25];
    assign key[863: 832] = S[26];
    assign key[895: 864] = S[27];
    assign key[927: 896] = S[28];
    assign key[959: 928] = S[29];
    assign key[991: 960] = S[30];
    assign key[1023: 992] = S[31];
    assign key[1055: 1024] = S[32];
    assign key[1087: 1056] = S[33];
    assign key[1119: 1088] = S[34];
    assign key[1151: 1120] = S[35];
    assign key[1183: 1152] = S[36];
    assign key[1215: 1184] = S[37];
    assign key[1247: 1216] = S[38];
    assign key[1279: 1248] = S[39];
    assign key[1311: 1280] = S[40];
    assign key[1343: 1312] = S[41];
    assign key[1375: 1344] = S[42];
    assign key[1407: 1376] = S[43];

    always @(posedge clk) begin
        if (rst) begin
            state <= PREPARE;
            
            for (gen_i = 0; gen_i < 44; gen_i = gen_i + 1) begin
                key_arr[gen_i] <= 0;
            end
            for (gen_i = 0; gen_i < 44; gen_i = gen_i + 1) begin
                S[gen_i] <= 0;
            end
            len_index <= 0;
            len_reg <= 0;
            pipeline_ctr <= 0;
            round_counter <= 0;
        end else begin
            valid <= state == KEY_YIELD;
            case (state)
                IDLE: begin
                    if (start) begin
                        state <= DATA_LOADING;
                        len_reg <= data;
                    end
                end
                DATA_LOADING: begin
                    if (valid_data && len_index < 176 && len_index < len_reg) begin
                        key_arr[len_index[7:2]][(len_index[1:0]) * 8 +: 8] <= data;
                        len_index <= len_index + 1;
                    end else if (valid_data) begin
                        if ((len_index + 1) >= len_reg) begin
                            state <= KEY_PROCESS;
                            rotSAB <= S[0] + A + B; // preparation for first rotation
                            len_reg <= len_reg[7:2] + (len_reg[1:0] > 0 ? 1 : 0); // becomes length in words, not bytes
                        end else len_index <= len_index + 1;
                    end
                end 
                KEY_PROCESS: begin
                    if (round_counter >= 8'd132) begin
                        state <= KEY_YIELD;
                    end else begin
                        pipeline_ctr <= pipeline_ctr + 1;
                        if (pipeline_ctr == 1'd0) begin
                            S[i] <= rot1;
                            A <= rot1;
                            ABrot <= rot1 + B;
                            rotLAB <= key_arr[j] + rot1 + B;
                        end else begin
                            key_arr[j] <= rot2;
                            B <= rot2;
                            if (i < 43)
                                rotSAB <= S[i + 1] + A + rot2;
                            else
                                rotSAB <= S[0] + A + rot2;
                            round_counter <= round_counter + 1;
                            i <= (i + 1) < 44 ? i + 1 : 0;
                            j <= (j + 1) < len_reg ? j + 1 : 0;
                        end
                    end
                end
                KEY_YIELD: begin
                    state <= PREPARE;
                end
                PREPARE: begin
                    A <= 0;
                    B <= 0;
                    i <= 0;
                    j <= 0;
                    round_counter <= 0;
                    len_index <= 0;
                    len_reg <= 0;
                    pipeline_ctr <= 0;
                    // generated according to rc6 specification using P32 = B7E15163 and Q32 = 9E3779B9 (hexadecimal)
                    S[0] <= 32'hb7e15163;
                    S[1] <= 32'h5618cb1c;
                    S[2] <= 32'hf45044d5;
                    S[3] <= 32'h9287be8e;
                    S[4] <= 32'h30bf3847;
                    S[5] <= 32'hcef6b200;
                    S[6] <= 32'h6d2e2bb9;
                    S[7] <= 32'h0b65a572;
                    S[8] <= 32'ha99d1f2b;
                    S[9] <= 32'h47d498e4;
                    S[10] <= 32'he60c129d;
                    S[11] <= 32'h84438c56;
                    S[12] <= 32'h227b060f;
                    S[13] <= 32'hc0b27fc8;
                    S[14] <= 32'h5ee9f981;
                    S[15] <= 32'hfd21733a;
                    S[16] <= 32'h9b58ecf3;
                    S[17] <= 32'h399066ac;
                    S[18] <= 32'hd7c7e065;
                    S[19] <= 32'h75ff5a1e;
                    S[20] <= 32'h1436d3d7;
                    S[21] <= 32'hb26e4d90;
                    S[22] <= 32'h50a5c749;
                    S[23] <= 32'heedd4102;
                    S[24] <= 32'h8d14babb;
                    S[25] <= 32'h2b4c3474;
                    S[26] <= 32'hc983ae2d;
                    S[27] <= 32'h67bb27e6;
                    S[28] <= 32'h05f2a19f;
                    S[29] <= 32'ha42a1b58;
                    S[30] <= 32'h42619511;
                    S[31] <= 32'he0990eca;
                    S[32] <= 32'h7ed08883;
                    S[33] <= 32'h1d08023c;
                    S[34] <= 32'hbb3f7bf5;
                    S[35] <= 32'h5976f5ae;
                    S[36] <= 32'hf7ae6f67;
                    S[37] <= 32'h95e5e920;
                    S[38] <= 32'h341d62d9;
                    S[39] <= 32'hd254dc92;
                    S[40] <= 32'h708c564b;
                    S[41] <= 32'h0ec3d004;
                    S[42] <= 32'hacfb49bd;
                    S[43] <= 32'h4b32c376;
                    state <= IDLE;
                end
            endcase
        end
        
    end

endmodule