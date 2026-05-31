module Multiplier (
    input  wire         clk,
    input  wire [255:0] op0,
    input  wire [31:0]  op1,
    output reg  [287:0] res
);
    always @(posedge clk) begin
        res <= op0 * op1;
    end
endmodule

// MU (p) = 129'h189fd9d74cac687728f0bd7aac5c44139
// MOD = 128'ha656c5fb97afa0eed886a53380706f2b

module Barrett_reduction #(
    BIT_WIDTH = 128
) (
    input  wire                   clk,
    input  wire                   rst,
    input  wire                   start,
    input  wire [2*BIT_WIDTH-1:0] op1,
    input  wire [BIT_WIDTH:0]     mu,
    input  wire [BIT_WIDTH-1:0]   mod,

    output reg  [255:0]           mult_op0,
    output reg  [31:0]            mult_op1,
    input  wire [287:0]           mult_res,

    output reg  [BIT_WIDTH-1:0]   res,
    output reg                    valid
);
    localparam 
        IDLE     = 1'd0,
        MULTIPLY = 1'd1;
    
    reg state;
    reg [2*BIT_WIDTH-1:0] op;
    reg [BIT_WIDTH:0] q_est;
    reg [2*BIT_WIDTH+32:0] q_est_acc;
    reg [2*BIT_WIDTH + 31:0] sub;
    reg [BIT_WIDTH+1:0] res_tmp;

    reg [BIT_WIDTH:0] mu_reg;
    reg [BIT_WIDTH-1:0] mod_reg;

    reg [7:0] mult_ctr;
    reg [7:0] mult_ctr_stage1;
    reg [7:0] mult_ctr_stage2;

    always @(posedge clk) begin
        if (rst) begin
            state <= IDLE;
            valid <= 1'b0;
            res <= '0;
            mult_ctr <= '0;
            q_est <= '0;
            q_est_acc <= '0;
        end else begin
            case (state)
                IDLE: begin
                    valid <= 0;
                    if (start) begin
                        op <= op1;
                        state <= MULTIPLY;
                        
                        mult_op0 <= op1;
                        mult_op1 <= mu[31:0];
                        mu_reg <= {32'd0, mu[128:32]}; 
                        mod_reg <= mod;
                        
                        q_est_acc <= '0;
                        mult_ctr_stage1 <= 0;
                        sub <= '0;
                    end
                end
                MULTIPLY: begin
                    if (mult_ctr == 0) begin
                        if (mult_ctr_stage1 < 4) begin
                            mult_op1 <= mu_reg[31:0];
                            mu_reg <= {32'd0, mu_reg[128:32]};
                        end else begin
                            mult_op1 <= 32'd0; // Nulový padding po odoslaní všetkého
                        end

                        if (mult_ctr_stage1 >= 1 && mult_ctr_stage1 <= 5) begin
                            q_est_acc <= (q_est_acc >> 32) + mult_res;
                        end

                        if (mult_ctr_stage1 == 6) begin
                            q_est <= q_est_acc[256:128];
                            mult_ctr <= 1;
                            mult_ctr_stage2 <= 0;

                            mult_op0 <= {127'd0, q_est_acc[256:128]};
                            mult_op1 <= mod_reg[31:0];
                            mod_reg <= {32'd0, mod_reg[127:32]};
                        end else begin
                            mult_ctr_stage1 <= mult_ctr_stage1 + 1;
                        end
                    
                    end else if (mult_ctr == 1) begin
                        if (mult_ctr_stage2 < 3) begin
                            mult_op1 <= mod_reg[31:0];
                            mod_reg <= {32'd0, mod_reg[127:32]};
                        end else begin
                            mult_op1 <= 32'd0;
                        end

                        if (mult_ctr_stage2 >= 1 && mult_ctr_stage2 <= 4) begin
                            sub <= (sub >> 32) + (mult_res << 96);
                        end

                        if (mult_ctr_stage2 == 5) begin
                            mult_ctr <= 2;
                        end else begin
                            mult_ctr_stage2 <= mult_ctr_stage2 + 1;
                        end
                    
                    end else if (mult_ctr == 2) begin
                        res_tmp <= op[BIT_WIDTH+1:0] - sub[BIT_WIDTH+1:0];
                        mult_ctr <= 3;
                    
                    end else if (mult_ctr == 3) begin
                        res <= (res_tmp >= {mod_reg, 1'b0} ? res_tmp - {mod_reg, 1'b0} : (res_tmp >= mod_reg ? res_tmp - mod_reg : res_tmp));
                        valid <= 1;
                        mult_ctr <= 0;
                        state <= IDLE;
                    end
                end
                default: state <= IDLE;
            endcase
        end
    end
endmodule

// P_PRIME = 0xfcc3c3c5
// Q_PRIME = 0x7f876909
module Montgomery_reduction #(
    parameter BIT_WIDTH = 128,
    parameter WORD_WIDTH = 32
) (
    input  wire                 clk,
    input  wire                 rst,
    input  wire                 start,
    input  wire [BIT_WIDTH-1:0] op0,
    input  wire [BIT_WIDTH-1:0] op1,
    input  wire [BIT_WIDTH-1:0] mod,
    input  wire [31:0]          mod_prime,

    output reg  [255:0]         mult_op0,
    output reg  [31:0]          mult_op1,
    input  wire [287:0]         mult_res,

    output reg  [BIT_WIDTH-1:0] res,
    output reg                  valid
);
    localparam PADDING_WIDTH = BIT_WIDTH - WORD_WIDTH;
    localparam [1:0]
        IDLE     = 2'd0,
        MULTIPLY = 2'd1,
        REDUCE   = 2'd2;
    
    reg [1:0] state; 

    reg [7:0] mult_ctr_stage;
    reg [7:0] reduction_ctr_stage;

    reg [BIT_WIDTH-1:0] tmp_op0, tmp_op1;
    reg [WORD_WIDTH-1:0] small_op0, small_op1;
    reg [2*BIT_WIDTH-1:0] tmp_res;

    always @(posedge clk) begin
        if (rst) begin
            state <= IDLE;
            valid <= 0;
            mult_ctr_stage <= 0;
            reduction_ctr_stage <= 0;
        end else begin
            case (state)
                IDLE: begin
                    valid <= 0;
                    if (start) begin
                        tmp_op0 <= op0;
                        tmp_op1 <= op1;
                        mult_ctr_stage <= 0;
                        reduction_ctr_stage <= 0;
                        tmp_res <= '0;
                        state <= MULTIPLY;
                    end
                end
                MULTIPLY: begin
                    if (mult_ctr_stage < 4) begin
                        mult_op0 <= {128'd0, tmp_op0};
                        mult_op1 <= tmp_op1[31:0];
                        tmp_op1  <= { {WORD_WIDTH{1'b0}}, tmp_op1[BIT_WIDTH-1 : WORD_WIDTH] };
                    end

                    if (mult_ctr_stage > 1 && mult_ctr_stage < 6) begin
                        tmp_res <= {mult_res[BIT_WIDTH+WORD_WIDTH-1:0], {PADDING_WIDTH{1'b0}}} + { {WORD_WIDTH{1'b0}}, tmp_res[2*BIT_WIDTH-1 : WORD_WIDTH] };
                    end

                    if (mult_ctr_stage == 5) begin
                        state <= REDUCE;
                        mult_ctr_stage <= 0;
                    end else begin
                        mult_ctr_stage <= mult_ctr_stage + 1;
                    end
                end 
                REDUCE: begin
                    if (reduction_ctr_stage < 4) begin
                        case (mult_ctr_stage)
                            0: begin
                                small_op0 <= mod_prime;
                                small_op1 <= tmp_res[31:0];
                            end
                            1: begin
                                mult_op0 <= mod;
                                mult_op1 <= small_op0 * small_op1;
                            end
                            3: begin
                                tmp_res[BIT_WIDTH+BIT_WIDTH-1:0] <= tmp_res[BIT_WIDTH+BIT_WIDTH-1:0] + mult_res[BIT_WIDTH+BIT_WIDTH-1:0];
                            end
                            4: begin
                                tmp_res <= {{WORD_WIDTH{1'b0}}, tmp_res[2*BIT_WIDTH-1:WORD_WIDTH]};
                                reduction_ctr_stage <= reduction_ctr_stage + 1;
                            end
                        endcase

                        if (mult_ctr_stage == 4) begin
                            mult_ctr_stage <= 0;
                        end else begin
                            mult_ctr_stage <= mult_ctr_stage + 1;
                        end
                    end else begin
                        reduction_ctr_stage <= 0;
                        res <= tmp_res[BIT_WIDTH:0] > mod ? tmp_res[BIT_WIDTH:0] - mod : tmp_res[BIT_WIDTH-1:0];
                        valid <= 1;
                        state <= IDLE;
                    end
                end 
                default: state <= IDLE;
            endcase
        end
    end
endmodule

module RSA #(
    parameter BIT_WIDTH = 128
) (
    input  wire                   clk,
    input  wire                   rst,
    input  wire                   start,
    input  wire [2*BIT_WIDTH-1:0] plain_text,
    input  wire                   cipher_flag,

    output reg  [2*BIT_WIDTH-1:0] cipher_text,
    output reg                    valid
);
    localparam MU_P = 129'h189fd9d74cac687728f0bd7aac5c44139;
    localparam MU_Q = 129'h1163786c39006b19d2f6e5561113c63fe;
    localparam P = 128'ha656c5fb97afa0eed886a53380706f2b;
    localparam Q = 128'heb8ea613fcdc5ebd0b4e7194754fcd2b;
    localparam Q_INV = 128'h17b60de317e25d4e8f575e7e887135f8;
    localparam P_PRIME = 32'h11b1287d;
    localparam Q_PRIME = 32'h38e5767d;
    localparam R_MOD_P = 128'h59a93a0468505f1127795acc7f8f90d5;
    localparam R_MOD_Q = 128'h147159ec0323a142f4b18e6b8ab032d5;
    localparam R_2_MOD_P = 128'h502e3ece7c7aab7af8ee58bca6d1546d;
    localparam R_2_MOD_Q = 128'h39faf04fa1171799f7d315b6cd66ce56;
    localparam D_MOD_P = 128'h6ee483fd0fca6b49e5af18cd004af4c7;
    localparam D_MOD_Q = 128'h9d09c40d533d947e0789a10da38a88c7;
    localparam E_MOD_P = 128'h00000000000000000000000000000003;
    localparam E_MOD_Q = 128'h00000000000000000000000000000003;

    localparam [2:0]
        IDLE       = 3'd0,
        SPLIT      = 3'd1,
        DOMAIN_IN  = 3'd2,
        CIPHER     = 3'd3,
        DOMAIN_OUT = 3'd4,
        MERGE      = 3'd5;

    reg [2:0] state;

    // ==========================================
    // Multipliers
    // ==========================================
    
    reg mult_inst_0_flag, mult_inst_1_flag;

    wire [287:0] mult_inst_0_res, mult_inst_1_res;

    // mont_inst_p, barrett_inst
    Multiplier mult_inst_0 (
        .clk(clk),
        .op0(mult_inst_0_flag ? mont_inst_p_mult_op0 : barrett_mult_op0),
        .op1(mult_inst_0_flag ? mont_inst_p_mult_op1 : barrett_mult_op1),
        .res(mult_inst_0_res)
    );

    // mont_inst_q
    Multiplier mult_inst_1 (
        .clk(clk),
        .op0(mult_inst_1_flag ? mont_inst_q_mult_op0 : garner_op0),
        .op1(mult_inst_1_flag ? mont_inst_q_mult_op1 : garner_op1),
        .res(mult_inst_1_res)
    );

    

    // ==========================================
    // Barrett module
    // ==========================================
    
    reg  barrett_start;
    wire [255:0] barrett_mult_op0;
    wire [31:0]  barrett_mult_op1;

    reg  [2*BIT_WIDTH-1:0] barrett_op;
    reg  [BIT_WIDTH:0]     barrett_mu;
    reg  [BIT_WIDTH-1:0]   barrett_mod;

    wire barrett_valid;
    wire [BIT_WIDTH-1:0] barrett_res;

    Barrett_reduction #(
        .BIT_WIDTH(BIT_WIDTH)
    ) barrett_inst (
        .clk(clk),
        .rst(rst),
        .start(barrett_start),
        .op1(barrett_op),
        .mu(barrett_mu),
        .mod(barrett_mod),
        
        // Komunikácia s násobičkou
        .mult_op0(barrett_mult_op0),
        .mult_op1(barrett_mult_op1),
        .mult_res(mult_inst_0_res),
        
        .res(barrett_res),
        .valid(barrett_valid)
    );

    // ==========================================
    // Parallel Montgomery modules
    // ==========================================
    
    wire [255:0] mont_inst_p_mult_op0;
    wire [31:0]  mont_inst_p_mult_op1;
    reg  mont_inst_p_start;
    
    reg  [BIT_WIDTH-1:0] mont_inst_p_op0;
    reg  [BIT_WIDTH-1:0] mont_inst_p_op1;
    reg  [BIT_WIDTH-1:0] mont_inst_p_mod;
    reg  [31:0]          mont_inst_p_mod_prime;

    wire mont_inst_p_valid;
    wire [BIT_WIDTH-1:0] mont_inst_p_res;

    // Montgomery pre vetvu P
    Montgomery_reduction #(
        .BIT_WIDTH(BIT_WIDTH),
        .WORD_WIDTH(32)
    ) mont_inst_p (
        .clk(clk),
        .rst(rst),
        .start(mont_inst_p_start),
        .op0(mont_inst_p_op0),
        .op1(mont_inst_p_op1),
        .mod(mont_inst_p_mod),
        .mod_prime(mont_inst_p_mod_prime),
        
        // Komunikácia s násobičkou
        .mult_op0(mont_inst_p_mult_op0),
        .mult_op1(mont_inst_p_mult_op1),
        .mult_res(mult_inst_0_res),
        
        .res(mont_inst_p_res),
        .valid(mont_inst_p_valid)
    );

    wire [255:0] mont_inst_q_mult_op0;
    wire [31:0]  mont_inst_q_mult_op1;
    reg  mont_inst_q_start;

    reg  [BIT_WIDTH-1:0] mont_inst_q_op0;
    reg  [BIT_WIDTH-1:0] mont_inst_q_op1;
    reg  [BIT_WIDTH-1:0] mont_inst_q_mod;
    reg  [31:0]          mont_inst_q_mod_prime;

    wire mont_inst_q_valid;
    wire [BIT_WIDTH-1:0] mont_inst_q_res;
    
    // Montgomery pre vetvu Q
    Montgomery_reduction #(
        .BIT_WIDTH(BIT_WIDTH),
        .WORD_WIDTH(32)
    ) mont_inst_q (
        .clk(clk),
        .rst(rst),
        .start(mont_inst_q_start),
        .op0(mont_inst_q_op0),
        .op1(mont_inst_q_op1),
        .mod(mont_inst_q_mod),
        .mod_prime(mont_inst_q_mod_prime),
        
        // Komunikácia s násobičkou
        .mult_op0(mont_inst_q_mult_op0),
        .mult_op1(mont_inst_q_mult_op1),
        .mult_res(mult_inst_1_res),
        
        .res(mont_inst_q_res),
        .valid(mont_inst_q_valid)
    );

    // ==========================================
    // FSM & Garner pomocné registre
    // ==========================================
    reg [7:0]   bit_ctr;         // Počítadlo bitov pre exponent (0 až 127)
    
    reg [BIT_WIDTH-1:0] m_p_res; 
    reg [BIT_WIDTH-1:0] m_q_res; 

    reg [255:0] garner_tmp_res;
    reg [2:0]   garner_ctr;      // Počítadlo fáz pre Garnerovo násobenie
    reg [255:0] garner_op0;
    reg [31:0]  garner_op1;

    reg barrett_state;
    reg [BIT_WIDTH-1:0] pt_0, pt_1, key_mod_p, key_mod_q, t_reg;
    reg [2:0] cipher_step;

    wire [BIT_WIDTH-1:0] m_q_mod_p = (m_q_res >= P) ? (m_q_res - P) : m_q_res;
    reg [127:0] mult_pp_0, mult_pp_1, mult_pp_2, mult_pp_3;

    always @(posedge clk) begin
        if (rst) begin
            state <= IDLE;
            mult_inst_0_flag <= 1'b0;
            mult_inst_1_flag <= 1'b0;
            barrett_start <= 1'b0;
            mont_inst_p_start <= 1'b0;
            mont_inst_q_start <= 1'b0;
            valid <= 1'b0; 
        end else begin
            case (state)
                IDLE: begin
                    valid <= 0;
                    if (start) begin
                        barrett_state <= 0;
                        barrett_start <= 1;
                        barrett_mod <= P;
                        barrett_mu <= MU_P;
                        barrett_op <= plain_text;
                        state <= SPLIT;
                        mult_inst_0_flag <= 1'b0;

                        key_mod_p <= cipher_flag ? D_MOD_P : E_MOD_P;
                        key_mod_q <= cipher_flag ? D_MOD_Q : E_MOD_Q;
                    end
                end
                SPLIT: begin
                    barrett_start <= 0;
                    if (barrett_state == 1'b0) begin
                        if (barrett_valid) begin
                            barrett_state <= 1'b1;
                            pt_0 <= barrett_res;
                            barrett_start <= 1;
                            barrett_mod <= Q;
                            barrett_mu <= MU_Q;
                            barrett_op <= plain_text;

                            $display("PT % P: %h\n", barrett_res);
                        end
                    end else begin
                        if (barrett_valid) begin
                            barrett_state <= 1'b0;
                            pt_1 <= barrett_res;
                            state <= DOMAIN_IN;
                            mult_inst_0_flag <= 1'b1;
                            mult_inst_1_flag <= 1'b1;
                            $display("PT % Q: %h\n", barrett_res);

                            mont_inst_p_start <= 1;
                            mont_inst_p_op0 <= R_2_MOD_P;
                            mont_inst_p_op1 <= pt_0;
                            mont_inst_p_mod <= P;
                            mont_inst_p_mod_prime <= P_PRIME;

                            mont_inst_q_start <= 1;
                            mont_inst_q_op0 <= R_2_MOD_Q;
                            mont_inst_q_op1 <= barrett_res;
                            mont_inst_q_mod <= Q;
                            mont_inst_q_mod_prime <= Q_PRIME;
                        end
                    end
                end
                DOMAIN_IN: begin
                    mont_inst_p_start <= 0;
                    mont_inst_q_start <= 0;
                    if (mont_inst_p_valid && mont_inst_q_valid) begin
                        $display("pt_p domain in: %h\n", mont_inst_p_res);
                        $display("pt_q domain in: %h\n", mont_inst_q_res);
                        pt_0 <= mont_inst_p_res;
                        pt_1 <= mont_inst_q_res;

                        mont_inst_p_start <= 1;
                        mont_inst_p_op0 <= R_MOD_P;
                        mont_inst_p_op1 <= R_MOD_P;
                        mont_inst_p_mod <= P;
                        mont_inst_p_mod_prime <= P_PRIME;

                        mont_inst_q_start <= 1;
                        mont_inst_q_op0 <= R_MOD_Q;
                        mont_inst_q_op1 <= R_MOD_Q;
                        mont_inst_q_mod <= Q;
                        mont_inst_q_mod_prime <= Q_PRIME;

                        state <= CIPHER;
                        cipher_step <= 3'd0;
                        bit_ctr <= 127;
                    end
                end
                CIPHER: begin
                    mont_inst_p_start <= 0;
                    mont_inst_q_start <= 0;
                    
                    case (cipher_step)
                        3'd0: begin
                            if (mont_inst_p_valid && mont_inst_q_valid) begin
                                $display("bit: %d", bit_ctr);
                                $display("pt_p after square: %h\n", mont_inst_p_res);
                                $display("pt_q after square: %h\n", mont_inst_q_res);
                                mont_inst_p_start <= 1;
                                mont_inst_p_op0 <= mont_inst_p_res;
                                mont_inst_p_op1 <= key_mod_p[BIT_WIDTH-1] ? pt_0 : R_MOD_P;
                                mont_inst_p_mod <= P;
                                mont_inst_p_mod_prime <= P_PRIME;
                                key_mod_p <= {key_mod_p[BIT_WIDTH-2:0], 1'b0};

                                mont_inst_q_start <= 1;
                                mont_inst_q_op0 <= mont_inst_q_res;
                                mont_inst_q_op1 <= key_mod_q[BIT_WIDTH-1] ? pt_1 : R_MOD_Q;
                                mont_inst_q_mod <= Q;
                                mont_inst_q_mod_prime <= Q_PRIME;
                                key_mod_q <= {key_mod_q[BIT_WIDTH-2:0], 1'b0};

                                cipher_step <= 3'd1;
                            end
                        end
                        3'd1: begin
                            if (mont_inst_p_valid && mont_inst_q_valid) begin
                                $display("pt_p after multiply: %h\n", mont_inst_p_res);
                                $display("pt_q after multiply: %h\n", mont_inst_q_res);
                                if (bit_ctr > 0) begin
                                    mont_inst_p_start <= 1;
                                    mont_inst_p_op0 <= mont_inst_p_res;
                                    mont_inst_p_op1 <= mont_inst_p_res;
                                    mont_inst_p_mod <= P;
                                    mont_inst_p_mod_prime <= P_PRIME;

                                    mont_inst_q_start <= 1;
                                    mont_inst_q_op0 <= mont_inst_q_res;
                                    mont_inst_q_op1 <= mont_inst_q_res;
                                    mont_inst_q_mod <= Q;
                                    mont_inst_q_mod_prime <= Q_PRIME;
                                    
                                    bit_ctr <= bit_ctr - 1;
                                    cipher_step <= 3'd0;
                                end else begin
                                    m_p_res <= mont_inst_p_res; 
                                    m_q_res <= mont_inst_q_res; 
                                    
                                    mont_inst_p_start <= 1;
                                    mont_inst_p_op0 <= mont_inst_p_res; 
                                    mont_inst_p_op1 <= 128'd1;          
                                    mont_inst_p_mod <= P;
                                    mont_inst_p_mod_prime <= P_PRIME;

                                    mont_inst_q_start <= 1;
                                    mont_inst_q_op0 <= mont_inst_q_res;
                                    mont_inst_q_op1 <= 128'd1;          
                                    mont_inst_q_mod <= Q;
                                    mont_inst_q_mod_prime <= Q_PRIME;
                                    
                                    state <= DOMAIN_OUT; 
                                end
                            end
                        end
                    endcase
                end
                DOMAIN_OUT: begin
                    mont_inst_p_start <= 0;
                    mont_inst_q_start <= 0;
                    if (mont_inst_p_valid && mont_inst_q_valid) begin
                        m_p_res <= mont_inst_p_res;
                        m_q_res <= mont_inst_q_res;
                        state <= MERGE;

                        mult_inst_0_flag <= 1'b0;
                        mult_inst_1_flag <= 1'b0;

                        cipher_step <= '0;
                    end
                end
                MERGE: begin
                    case (cipher_step)
                        3'd0: begin
                            t_reg <= (m_p_res < m_q_mod_p) ? (m_p_res + P - m_q_mod_p) : (m_p_res - m_q_mod_p);
                            cipher_step <= 3'd1;
                        end
                        
                        3'd1: begin
                            mult_pp_0 <= t_reg[127:64] * Q_INV[127:64]; // Horné * Horné (HH)
                            mult_pp_1 <= t_reg[127:64] * Q_INV[63:0];   // Horné * Dolné (HL)
                            mult_pp_2 <= t_reg[63:0]   * Q_INV[127:64]; // Dolné * Horné (LH)
                            mult_pp_3 <= t_reg[63:0]   * Q_INV[63:0];   // Dolné * Dolné (LL)
                            cipher_step <= 3'd2;
                        end
                        3'd2: begin
                            barrett_start <= 1;
                            barrett_op <= {mult_pp_0, 128'd0} + 
                                          {64'd0, mult_pp_1, 64'd0} + 
                                          {64'd0, mult_pp_2, 64'd0} + 
                                          {128'd0, mult_pp_3};
                            barrett_mod <= P;
                            barrett_mu <= MU_P;
                            cipher_step <= 3'd3;
                        end
                        3'd3: begin
                            barrett_start <= 0;
                            if (barrett_valid) begin
                                mult_pp_0 <= barrett_res[127:64] * Q[127:64];
                                mult_pp_1 <= barrett_res[127:64] * Q[63:0];
                                mult_pp_2 <= barrett_res[63:0]   * Q[127:64];
                                mult_pp_3 <= barrett_res[63:0]   * Q[63:0];
                                cipher_step <= 3'd4;
                            end
                        end
                        
                        3'd4: begin
                            cipher_text <= {128'd0, m_q_res} + 
                                           {mult_pp_0, 128'd0} + 
                                           {64'd0, mult_pp_1, 64'd0} + 
                                           {64'd0, mult_pp_2, 64'd0} + 
                                           {128'd0, mult_pp_3};
                            valid <= 1;
                            state <= IDLE;
                        end
                    endcase
                end
                default: state <= IDLE;
            endcase
        end
    end

endmodule