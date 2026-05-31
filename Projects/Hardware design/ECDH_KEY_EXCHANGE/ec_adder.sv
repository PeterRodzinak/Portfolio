// returns multiplication of two operands in GF(2^79)
module MULTIPLIER (
    input  wire        clk,
    input  wire        rst,
    input  wire        start,
    input  wire [78:0] OP1,
    input  wire [78:0] OP2,

    output reg  [78:0] RES,
    output wire        valid
);

    localparam [1:0]
        IDLE         = 2'd0,
        COUNTING     = 2'd1,
        RESULT       = 2'd2;
    
    reg [1:0] state;
    reg [7:0] pipeline_counter;

    reg [78:0] OP_A, OP_B;

    assign valid = state == RESULT;

    always @(posedge clk) begin
        if (rst) begin
            state <= IDLE;
            pipeline_counter <= 0;
            RES <= 0;
            OP_A <= 0;
            OP_B <= 0;
        end else begin
            case (state)
                IDLE: begin
                    if (start) begin
                        OP_A <= OP1;
                        OP_B <= OP2;
                        RES <= 0;
                        pipeline_counter <= 0;
                        state <= COUNTING;
                    end
                end
                
                COUNTING: begin
                    RES <= OP_A[0] ? (RES ^ OP_B) : RES;
                    OP_A <= OP_A >> 1;
                    OP_B <= {OP_B[77:9], OP_B[8] ^ OP_B[78], OP_B[7:0], OP_B[78]};
                    
                    if (pipeline_counter == 8'd78) begin
                        state <= RESULT;
                    end else begin
                        pipeline_counter <= pipeline_counter + 1;
                    end
                end
                RESULT: begin
                    state <= IDLE;
                end
            endcase
        end
    end

endmodule

module MULTIPLIER2 (
    input  wire        clk,
    input  wire        rst,
    input  wire        start,
    input  wire [78:0] OP1,
    input  wire [78:0] OP2,

    output reg  [78:0] RES,
    output reg         valid
);
    import gf_math_pkg::*;

    localparam 
        IDLE     = 1'd0,
        COUNTING = 1'd1;

    reg       state;
    reg [7:0] pipeline_counter;

    reg [86:0] accumulator;

    reg [79:0] mult_op;
    reg [78:0] op;

    wire [85:0] tmp_res = DIGIT_8_MULT(op, mult_op[79:72]);
    wire [86:0] acc_tmp1 = {accumulator[86], tmp_res ^ accumulator[85:0]};
    wire [86:0] acc_tmp2 = {acc_tmp1[86:17], acc_tmp1[86:79] ^ acc_tmp1[16:9], acc_tmp1[8], acc_tmp1[86:79] ^ acc_tmp1[7:0]};

    always @(posedge clk) begin
        if (rst) begin
            state <= IDLE;
            pipeline_counter <= 0;
            valid <= 0;
            accumulator <= 87'd0;
            op <= 79'd0;
            mult_op <= 80'd0;
        end else begin
            case (state)
                IDLE: begin
                    accumulator <= 87'd0;
                    valid <= 0;
                    if (start) begin
                        state <= COUNTING;
                        op <= OP1;
                        mult_op <= {1'd0, OP2};
                        pipeline_counter <= 0;
                    end
                end 
                COUNTING: begin
                    if (pipeline_counter == 9) begin
                        state <= IDLE;
                        valid <= 1;
                    end else begin
                        valid <= 0;
                    end
                    RES <= acc_tmp2[78:0];
                    mult_op <= {mult_op[71:0], 8'd0};
                    accumulator <= {acc_tmp2[78:0], 8'd0};
                    pipeline_counter <= pipeline_counter + 1;
                end
            endcase
        end
    end
    
endmodule

// returns polynomial inversion, works using Itoh-Tsujii Algorithm
module INVERTOR (
    input  wire        clk,
    input  wire        rst,
    input  wire        start,
    input  wire [78:0] OP,

    output reg         mult_start,
    output reg  [78:0] MULT_OP1,
    output reg  [78:0] MULT_OP2,
    input  wire [78:0] MULT_RES,
    input  wire        mult_valid,

    output wire [78:0] RES,
    output wire        valid
);
    import gf_math_pkg::*;

    localparam [3:0] 
        IDLE     = 4'd0,
        BIT_2    = 4'd1,
        BIT_4    = 4'd2,
        BIT_8    = 4'd3,
        BIT_16   = 4'd4,
        BIT_32   = 4'd5,
        BIT_64   = 4'd6,
        BIT_72   = 4'd7,
        BIT_76   = 4'd8,
        BIT_78   = 4'd9,
        BIT_79   = 4'd10;

    reg [3:0] state;

    reg [78:0] L1, L2, L4, L8, L_TEMP;

    reg mult_stage;
    assign valid = state == BIT_79;
    assign RES = POWER_1_BIT(L_TEMP);

    always @(posedge clk) begin
        if (rst) begin
            L1 <= 0;
            L2 <= 0;
            L4 <= 0;
            L8 <= 0;
            L_TEMP <= 0;
            mult_start <= 0;
            mult_stage <= 0;
            state <= IDLE;
        end else begin
            case (state)
                IDLE: begin
                    if (start) begin
                        L1 <= OP;
                        mult_stage <= 0;
                        state <= BIT_2;
                    end
                end
                BIT_2: begin
                    if (mult_stage == 0) begin
                        MULT_OP1 <= L1;
                        MULT_OP2 <= POWER_1_BIT(L1);
                        mult_start <= 1;
                        mult_stage <= 1;
                    end else begin
                        mult_start <= 0;
                        if (mult_valid) begin
                            L2 <= MULT_RES;
                            mult_stage <= 0;
                            state <= BIT_4;
                        end
                    end
                end
                BIT_4: begin
                    if (mult_stage == 0) begin
                        MULT_OP1 <= L2;
                        MULT_OP2 <= POWER_2_BIT(L2);
                        mult_start <= 1;
                        mult_stage <= 1;
                    end else begin
                        mult_start <= 0;
                        if (mult_valid) begin
                            L4 <= MULT_RES;
                            mult_stage <= 0;
                            state <= BIT_8;
                        end
                    end
                end
                BIT_8: begin
                    if (mult_stage == 0) begin
                        MULT_OP1 <= L4;
                        MULT_OP2 <= POWER_4_BIT(L4);
                        mult_start <= 1;
                        mult_stage <= 1;
                    end else begin
                        mult_start <= 0;
                        if (mult_valid) begin
                            L8 <= MULT_RES;
                            mult_stage <= 0;
                            state <= BIT_16;
                        end
                    end
                end
                BIT_16: begin
                    if (mult_stage == 0) begin
                        MULT_OP1 <= L8;
                        MULT_OP2 <= POWER_8_BIT(L8);
                        mult_start <= 1;
                        mult_stage <= 1;
                    end else begin
                        mult_start <= 0;
                        if (mult_valid) begin
                            L_TEMP <= MULT_RES;
                            mult_stage <= 0;
                            state <= BIT_32;
                        end
                    end
                end
                BIT_32: begin
                    if (mult_stage == 0) begin
                        MULT_OP1 <= L_TEMP;
                        MULT_OP2 <= POWER_16_BIT(L_TEMP);
                        mult_start <= 1;
                        mult_stage <= 1;
                    end else begin
                        mult_start <= 0;
                        if (mult_valid) begin
                            L_TEMP <= MULT_RES;
                            mult_stage <= 0;
                            state <= BIT_64;
                        end
                    end
                end
                BIT_64: begin
                    if (mult_stage == 0) begin
                        MULT_OP1 <= L_TEMP;
                        MULT_OP2 <= POWER_32_BIT(L_TEMP);
                        mult_start <= 1;
                        mult_stage <= 1;
                    end else begin
                        mult_start <= 0;
                        if (mult_valid) begin
                            L_TEMP <= MULT_RES;
                            mult_stage <= 0;
                            state <= BIT_72;
                        end
                    end
                end
                BIT_72: begin
                    if (mult_stage == 0) begin
                        MULT_OP1 <= L8;
                        MULT_OP2 <= POWER_8_BIT(L_TEMP);
                        mult_start <= 1;
                        mult_stage <= 1;
                    end else begin
                        mult_start <= 0;
                        if (mult_valid) begin
                            L_TEMP <= MULT_RES;
                            mult_stage <= 0;
                            state <= BIT_76;
                        end
                    end
                end
                BIT_76: begin
                    if (mult_stage == 0) begin
                        MULT_OP1 <= L4;
                        MULT_OP2 <= POWER_4_BIT(L_TEMP);
                        mult_start <= 1;
                        mult_stage <= 1;
                    end else begin
                        mult_start <= 0;
                        if (mult_valid) begin
                            L_TEMP <= MULT_RES;
                            mult_stage <= 0;
                            state <= BIT_78;
                        end
                    end
                end
                BIT_78: begin
                    if (mult_stage == 0) begin
                        MULT_OP1 <= L2;
                        MULT_OP2 <= POWER_2_BIT(L_TEMP);
                        mult_start <= 1;
                        mult_stage <= 1;
                    end else begin
                        mult_start <= 0;
                        if (mult_valid) begin
                            L_TEMP <= MULT_RES;
                            mult_stage <= 0;
                            state <= BIT_79;
                        end
                    end
                end
                BIT_79: begin
                    state <= IDLE;
                end
            endcase
        end
    end
    
endmodule

module ECC_ADD (
    input  wire        clk,
    input  wire        rst,
    input  wire        start,
    input  wire [78:0] OP1_X,
    input  wire [78:0] OP1_Y,
    input  wire        is_inf_1,
    input  wire [78:0] OP2_X,
    input  wire [78:0] OP2_Y,
    input  wire        is_inf_2,

    output reg         mult_start,
    output reg  [78:0] mult_op1,
    output reg  [78:0] mult_op2,
    input  wire [78:0] mult_res,
    input  wire        mult_valid,

    output reg         inv_start,
    output reg  [78:0] inv_op,
    input  wire        inv_valid,
    input  wire [78:0] inv_res,

    output reg  [78:0] RES_X,
    output reg  [78:0] RES_Y,
    output reg         is_inf_res,
    output reg         valid
);

    import gf_math_pkg::*;

    parameter ECC_A = 79'h4A2E38A8F66D7F4C385F;
    parameter ECC_B = 79'h2C0BB31C6BECC03D68A7;
    
    localparam [2:0] 
        IDLE    = 3'd0,
        STAGE3  = 3'd1,
        STAGE6  = 3'd2,
        STAGE7  = 3'd3,
        PREPARE = 3'd4;

    reg [2:0] state;
    reg [2:0] stage3_ctr, stage6_ctr, stage7_ctr;

    reg [78:0] tmp_y1y2, alpha, op1_x_tmp, op1_y_tmp, op2_x_tmp, op2_y_tmp;

    always @(posedge clk) begin
        if (rst) begin
            state <= PREPARE;
            inv_start <= 0;
            mult_start <= 0;
        end else begin
            case (state)
                IDLE: begin
                    if (start) begin
                        if (is_inf_1 && is_inf_2) begin
                            is_inf_res <= 1;
                            valid <= 1;
                            state <= PREPARE;
                        end else if (is_inf_1) begin
                            RES_X <= OP2_X;
                            RES_Y <= OP2_Y;
                            valid <= 1;
                            state <= PREPARE;
                        end else if (is_inf_2) begin
                            RES_X <= OP1_X;
                            RES_Y <= OP1_Y;
                            valid <= 1;
                            state <= PREPARE;
                        end else begin
                            op1_x_tmp <= OP1_X;
                            op1_y_tmp <= OP1_Y;
                            op2_x_tmp <= OP2_X;
                            op2_y_tmp <= OP2_Y;
                            
                            if (OP1_X == OP2_X) begin
                                if (OP1_Y != OP2_Y) begin
                                    is_inf_res <= 1;
                                    valid <= 1;
                                    state <= PREPARE;
                                end else if (OP2_X == 79'd0) begin
                                    is_inf_res <= 1;
                                    valid <= 1;
                                    state <= PREPARE;
                                end else begin
                                    state <= STAGE6;
                                    inv_op <= OP2_X;
                                    inv_start <= 1;
                                end
                            end else begin
                                inv_start <= 1;
                                inv_op <= OP1_X ^ OP2_X;
                                tmp_y1y2 <= OP1_Y ^ OP2_Y;
                                state <= STAGE3;
                            end
                        end
                    end
                end
                STAGE3: begin
                    if (stage3_ctr == 0) begin
                        inv_start <= 0;
                        if (inv_valid) begin
                            mult_start <= 1;
                            mult_op1 <= tmp_y1y2;
                            mult_op2 <= inv_res;
                            stage3_ctr <= 1;
                        end
                    end else if (stage3_ctr == 1) begin
                        mult_start <= 0;
                        if (mult_valid) begin
                            alpha <= mult_res;
                            stage3_ctr <= 2;
                        end
                    end else if (stage3_ctr == 2) begin
                        stage3_ctr <= 0;
                        RES_X <= ECC_A ^ POWER_1_BIT(alpha) ^ alpha ^ op1_x_tmp ^ op2_x_tmp;
                        state <= STAGE7;
                    end
                end
                STAGE6: begin
                    if (stage6_ctr == 0) begin
                        inv_start <= 0;
                        if (inv_valid) begin
                            mult_start <= 1;
                            stage6_ctr <= 1;
                            mult_op1 <= inv_res;
                            mult_op2 <= op2_y_tmp;
                        end
                    end else if (stage6_ctr == 1) begin
                        mult_start <= 0;
                        if (mult_valid) begin
                            alpha <= mult_res ^ op2_x_tmp;
                            RES_X <= ECC_A ^ POWER_1_BIT(mult_res ^ op2_x_tmp) ^ mult_res ^ op2_x_tmp;
                            state <= STAGE7;
                            stage6_ctr <= 0;
                        end
                    end
                end
                STAGE7: begin
                    if (stage7_ctr == 0) begin
                        mult_op1 <= alpha;
                        mult_op2 <= RES_X ^ op2_x_tmp;
                        mult_start <= 1;
                        stage7_ctr <= 1;
                    end else if (stage7_ctr == 1) begin
                        mult_start <= 0;
                        if (mult_valid) begin
                            RES_Y <= mult_res ^ RES_X ^ op2_y_tmp;
                            valid <= 1;
                            stage7_ctr <= 0;
                            state <= PREPARE;
                        end
                    end 
                end
                PREPARE: begin
                    valid <= 0;
                    stage3_ctr <= 0;
                    stage6_ctr <= 0;
                    stage7_ctr <= 0;
                    is_inf_res <= 0;
                    tmp_y1y2 <= 79'd0;
                    state <= IDLE;
                    alpha <= 79'd0;
                    op1_x_tmp <= 79'd0;
                    op1_y_tmp <= 79'd0;
                    op2_x_tmp <= 79'd0;
                    op2_y_tmp <= 79'd0;
                end
            endcase
        end
    end

endmodule

module ECC_DOUBLE (
    input  wire        clk,
    input  wire        rst,
    input  wire        start,
    input  wire [78:0] OP_X,
    input  wire [78:0] OP_Y,
    input  wire        is_inf,

    input  wire        mult_valid,
    input  wire [78:0] mult_result,
    output reg         mult_start,
    output reg  [78:0] mult_op1,
    output reg  [78:0] mult_op2,

    input  wire        inv_valid,
    input  wire [78:0] inv_result,
    output reg         inv_start,
    output reg  [78:0] inv_op,

    output reg  [78:0] RES_X,
    output reg  [78:0] RES_Y,
    output reg         is_inf_res,
    output reg         valid
);
    import gf_math_pkg::*;

    localparam [1:0]
        IDLE       = 2'd0,
        ALPHA_INV  = 2'd1,
        ALPHA_MULT = 2'd2,
        RESULT     = 2'd3;

    localparam [78:0] A_COEFF = 79'h4A2E38A8F66D7F4C385F;

    reg [1:0] state;

    reg [78:0] OP_Y_tmp;
    reg [78:0] OP_X_tmp;

    wire [78:0] lambda        = OP_X_tmp ^ mult_result;
    wire [78:0] lambda_sq     = POWER_1_BIT(lambda);
    wire [78:0] next_x        = lambda_sq ^ lambda ^ A_COEFF;
    
    wire [78:0] lambda_plus_1 = {lambda[78:1], ~lambda[0]};

    always @(posedge clk) begin
        if (rst) begin
            state <= IDLE;
            valid <= 0;
        end else begin
            case (state)
                IDLE: begin
                    valid <= 0;
                    if (start) begin
                        if (is_inf == 1 || OP_X == 0) begin
                            is_inf_res <= 1;
                            RES_X <= 79'd0;
                            RES_Y <= 79'd0;
                            valid <= 1;
                        end else begin
                            is_inf_res <= 0;
                            inv_start <= 1;
                            inv_op <= OP_X;
                            OP_Y_tmp <= OP_Y;
                            OP_X_tmp <= OP_X;
                            state <= ALPHA_INV;
                        end
                    end
                end 
                ALPHA_INV: begin
                    inv_start <= 0;
                    if (inv_valid) begin
                        mult_start <= 1;
                        mult_op1 <= OP_Y_tmp;
                        mult_op2 <= inv_result;
                        state <= ALPHA_MULT;
                    end
                end 
                ALPHA_MULT: begin
                    mult_start <= 0;
                    if (mult_valid) begin
                        RES_X <= next_x;
                        mult_start <= 1;
                        mult_op1 <= lambda_plus_1;
                        mult_op2 <= next_x;
                        state <= RESULT;
                    end
                end 
                RESULT: begin
                    valid <= 0;
                    mult_start <= 0;
                    if (mult_valid) begin
                        RES_Y <= mult_result ^ POWER_1_BIT(OP_X_tmp);
                        valid <= 1;
                        state <= IDLE;
                    end
                end 
            endcase
        end
    end
endmodule

module ECDH_CORE (
    input  wire        clk,
    input  wire        rst,
    input  wire        start,

    input  wire [78:0] key,
    input  wire [78:0] p_x,
    input  wire [78:0] p_y,
    input  wire        p_inf,

    output reg  [78:0] res_x,
    output reg  [78:0] res_y,
    output reg         res_inf,
    output reg         valid
);

    localparam [1:0]
        IDLE        = 2'd0,
        CALCULATING = 2'd1;

    reg [1:0] state;
    reg [6:0] bit_ctr;
    reg       bit_phase;

    reg [78:0] key_reg, point_x, point_y;
    reg point_inf;

    wire inv_mult_start, add_mult_start, double_mult_start;
    wire mult_start = inv_mult_start || add_mult_start || double_mult_start;

    wire [78:0] inv_mult_op1, inv_mult_op2, add_mult_op1, add_mult_op2, double_mult_op1, double_mult_op2;
    wire [78:0] mult_op1 = inv_mult_start ? inv_mult_op1 : add_mult_start ? add_mult_op1 : double_mult_start ? double_mult_op1 : 79'd0;
    wire [78:0] mult_op2 = inv_mult_start ? inv_mult_op2 : add_mult_start ? add_mult_op2 : double_mult_start ? double_mult_op2 : 79'd0;
    wire [78:0] mult_res;
    wire mult_valid;

    wire add_inv_start, double_inv_start;
    wire inv_start = add_inv_start || double_inv_start;

    wire [78:0] add_inv_op, double_inv_op;
    wire [78:0] inv_op = add_inv_start ? add_inv_op : double_inv_start ? double_inv_op : 79'd0;
    wire [78:0] inv_res;
    wire inv_valid;

    reg add_start, double_start;
    reg add_p_inf, add_q_inf, double_inf;
    reg [78:0] add_p_x, add_p_y, add_q_x, add_q_y, double_op_x, double_op_y;
    wire [78:0] add_res_x, add_res_y, double_res_x, double_res_y;
    wire add_valid, double_valid, add_res_inf, double_res_inf;
    
    MULTIPLIER2 shared_mult (
        .clk        (clk),
        .rst        (rst),
        .start      (mult_start),
        .OP1        (mult_op1),
        .OP2        (mult_op2),
        .RES        (mult_res),
        .valid      (mult_valid)
    );

    INVERTOR shared_inv (
        .clk        (clk),
        .rst        (rst),
        .start      (inv_start),
        .OP         (inv_op),
        .mult_start (inv_mult_start),
        .MULT_OP1   (inv_mult_op1),
        .MULT_OP2   (inv_mult_op2),
        .MULT_RES   (mult_res),
        .mult_valid (mult_valid),
        .RES        (inv_res),
        .valid      (inv_valid)
    );

    ECC_DOUBLE ecc_double_inst (
        .clk        (clk),
        .rst        (rst),
        .start      (double_start),
        .OP_X       (double_op_x),
        .OP_Y       (double_op_y),
        .is_inf     (double_inf),
        .mult_start (double_mult_start),
        .mult_op1   (double_mult_op1),
        .mult_op2   (double_mult_op2),
        .mult_result(mult_res),
        .mult_valid (mult_valid),
        .inv_valid  (inv_valid),
        .inv_result (inv_res),
        .inv_start  (double_inv_start),
        .inv_op     (double_inv_op),
        .RES_X      (double_res_x),
        .RES_Y      (double_res_y),
        .is_inf_res (double_res_inf),
        .valid      (double_valid)
    );

    ECC_ADD ecc_add_inst (
        .clk        (clk),
        .rst        (rst),
        .start      (add_start),
        .OP1_X      (add_p_x),
        .OP1_Y      (add_p_y),
        .is_inf_1   (add_p_inf),
        .OP2_X      (add_q_x),
        .OP2_Y      (add_q_y),
        .is_inf_2   (add_q_inf),
        .mult_start (add_mult_start),
        .mult_op1   (add_mult_op1),
        .mult_op2   (add_mult_op2),
        .mult_res   (mult_res),
        .mult_valid (mult_valid),
        .inv_start  (add_inv_start),
        .inv_op     (add_inv_op),
        .inv_valid  (inv_valid),
        .inv_res    (inv_res),
        .RES_X      (add_res_x),
        .RES_Y      (add_res_y),
        .is_inf_res (add_res_inf),
        .valid      (add_valid)
    );

    always @(posedge clk) begin
        if (rst) begin
            state <= IDLE;
            bit_ctr <= 0;
            bit_phase <= 1;
        end else begin
            case (state)
                IDLE: begin
                    valid <= 0;
                    if (start) begin
                        bit_ctr <= 0;
                        bit_phase <= 1;
                        state <= CALCULATING;
                        key_reg <= key;
                        point_x <= p_x;
                        point_y <= p_y;
                        point_inf <= p_inf;

                        double_start <= 1;
                        double_op_x <= 0;
                        double_op_y <= 0;
                        double_inf <= 1;
                    end
                end
                CALCULATING: begin
                    if (bit_phase == 0) begin
                        add_start <= 0;
                        if (add_valid) begin
                            if (bit_ctr == 7'd78) begin
                                res_x <= add_res_x;
                                res_y <= add_res_y;
                                res_inf <= add_res_inf;
                                valid <= 1;
                                state <= IDLE;
                            end else begin
                                bit_phase <= 1;
                                bit_ctr <= bit_ctr + 1;
                                double_start <= 1;
                                double_op_x <= add_res_x;
                                double_op_y <= add_res_y;
                                double_inf  <= add_res_inf;
                            end
                        end
                    end else if (bit_phase == 1) begin
                        double_start <= 0;
                        if (double_valid) begin
                            if (key_reg[78]) begin
                                bit_phase <= 0;
                                add_start <= 1;
                                add_p_x <= double_res_x;
                                add_p_y <= double_res_y;
                                add_p_inf <= double_res_inf;
                                add_q_x <= point_x;
                                add_q_y <= point_y;
                                add_q_inf <= point_inf;
                            end else begin
                                if (bit_ctr == 7'd78) begin
                                    res_x <= double_res_x;
                                    res_y <= double_res_y;
                                    res_inf <= double_res_inf;
                                    valid <= 1;
                                    state <= IDLE;
                                end else begin
                                    double_start <= 1;
                                    double_op_x <= double_res_x;
                                    double_op_y <= double_res_y;
                                    double_inf  <= double_res_inf;
                                    bit_ctr <= bit_ctr + 1;
                                end
                            end
                            key_reg <= {key_reg[77:0], 1'b0};
                        end
                    end
                end
            endcase
        end
    end
    
endmodule