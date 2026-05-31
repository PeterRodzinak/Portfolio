package gf_math_pkg;
    function [78:0] POWER_1_BIT;
        input [78:0] OP;
        begin
            POWER_1_BIT[0]  = OP[0];
            POWER_1_BIT[1]  = OP[40] ^ OP[75];
            POWER_1_BIT[2]  = OP[1];
            POWER_1_BIT[3]  = OP[41] ^ OP[76];
            POWER_1_BIT[4]  = OP[2];
            POWER_1_BIT[5]  = OP[42] ^ OP[77];
            POWER_1_BIT[6]  = OP[3];
            POWER_1_BIT[7]  = OP[43] ^ OP[78];
            POWER_1_BIT[8]  = OP[4];
            POWER_1_BIT[9]  = OP[44];
            POWER_1_BIT[10] = OP[5] ^ OP[40] ^ OP[75];
            POWER_1_BIT[11] = OP[45];
            POWER_1_BIT[12] = OP[6] ^ OP[41] ^ OP[76];
            POWER_1_BIT[13] = OP[46];
            POWER_1_BIT[14] = OP[7] ^ OP[42] ^ OP[77];
            POWER_1_BIT[15] = OP[47];
            POWER_1_BIT[16] = OP[8] ^ OP[43] ^ OP[78];
            POWER_1_BIT[17] = OP[48];
            POWER_1_BIT[18] = OP[9] ^ OP[44];
            POWER_1_BIT[19] = OP[49];
            POWER_1_BIT[20] = OP[10] ^ OP[45];
            POWER_1_BIT[21] = OP[50];
            POWER_1_BIT[22] = OP[11] ^ OP[46];
            POWER_1_BIT[23] = OP[51];
            POWER_1_BIT[24] = OP[12] ^ OP[47];
            POWER_1_BIT[25] = OP[52];
            POWER_1_BIT[26] = OP[13] ^ OP[48];
            POWER_1_BIT[27] = OP[53];
            POWER_1_BIT[28] = OP[14] ^ OP[49];
            POWER_1_BIT[29] = OP[54];
            POWER_1_BIT[30] = OP[15] ^ OP[50];
            POWER_1_BIT[31] = OP[55];
            POWER_1_BIT[32] = OP[16] ^ OP[51];
            POWER_1_BIT[33] = OP[56];
            POWER_1_BIT[34] = OP[17] ^ OP[52];
            POWER_1_BIT[35] = OP[57];
            POWER_1_BIT[36] = OP[18] ^ OP[53];
            POWER_1_BIT[37] = OP[58];
            POWER_1_BIT[38] = OP[19] ^ OP[54];
            POWER_1_BIT[39] = OP[59];
            POWER_1_BIT[40] = OP[20] ^ OP[55];
            POWER_1_BIT[41] = OP[60];
            POWER_1_BIT[42] = OP[21] ^ OP[56];
            POWER_1_BIT[43] = OP[61];
            POWER_1_BIT[44] = OP[22] ^ OP[57];
            POWER_1_BIT[45] = OP[62];
            POWER_1_BIT[46] = OP[23] ^ OP[58];
            POWER_1_BIT[47] = OP[63];
            POWER_1_BIT[48] = OP[24] ^ OP[59];
            POWER_1_BIT[49] = OP[64];
            POWER_1_BIT[50] = OP[25] ^ OP[60];
            POWER_1_BIT[51] = OP[65];
            POWER_1_BIT[52] = OP[26] ^ OP[61];
            POWER_1_BIT[53] = OP[66];
            POWER_1_BIT[54] = OP[27] ^ OP[62];
            POWER_1_BIT[55] = OP[67];
            POWER_1_BIT[56] = OP[28] ^ OP[63];
            POWER_1_BIT[57] = OP[68];
            POWER_1_BIT[58] = OP[29] ^ OP[64];
            POWER_1_BIT[59] = OP[69];
            POWER_1_BIT[60] = OP[30] ^ OP[65];
            POWER_1_BIT[61] = OP[70];
            POWER_1_BIT[62] = OP[31] ^ OP[66];
            POWER_1_BIT[63] = OP[71];
            POWER_1_BIT[64] = OP[32] ^ OP[67];
            POWER_1_BIT[65] = OP[72];
            POWER_1_BIT[66] = OP[33] ^ OP[68];
            POWER_1_BIT[67] = OP[73];
            POWER_1_BIT[68] = OP[34] ^ OP[69];
            POWER_1_BIT[69] = OP[74];
            POWER_1_BIT[70] = OP[35] ^ OP[70];
            POWER_1_BIT[71] = OP[75];
            POWER_1_BIT[72] = OP[36] ^ OP[71];
            POWER_1_BIT[73] = OP[76];
            POWER_1_BIT[74] = OP[37] ^ OP[72];
            POWER_1_BIT[75] = OP[77];
            POWER_1_BIT[76] = OP[38] ^ OP[73];
            POWER_1_BIT[77] = OP[78];
            POWER_1_BIT[78] = OP[39] ^ OP[74];
        end
    endfunction

    function [78:0] POWER_2_BIT;
        input [78:0] op;
        begin
            POWER_2_BIT = POWER_1_BIT(POWER_1_BIT(op));
        end
    endfunction

    function [78:0] POWER_4_BIT;
        input [78:0] op;
        begin
            POWER_4_BIT = POWER_2_BIT(POWER_2_BIT(op));
        end
    endfunction

    function [78:0] POWER_8_BIT;
        input [78:0] op;
        begin
            POWER_8_BIT = POWER_4_BIT(POWER_4_BIT(op));
        end
    endfunction

    function [78:0] POWER_16_BIT;
        input [78:0] op;
        begin
            POWER_16_BIT = POWER_8_BIT(POWER_8_BIT(op));
        end
    endfunction

    function [78:0] POWER_32_BIT;
        input [78:0] op;
        begin
            POWER_32_BIT = POWER_16_BIT(POWER_16_BIT(op));
        end
    endfunction

    function [85:0] DIGIT_8_MULT;
        input [78:0] op;
        input [7:0]  mult_op;
        reg [85:0] b0;
        reg [85:0] b1;
        reg [85:0] b2;
        reg [85:0] b3;
        reg [85:0] b4;
        reg [85:0] b5;
        reg [85:0] b6;
        reg [85:0] b7;
        begin
            b0 = mult_op[0] ? {7'd0, op      } : 86'd0;
            b1 = mult_op[1] ? {6'd0, op, 1'd0} : 86'd0; 
            b2 = mult_op[2] ? {5'd0, op, 2'd0} : 86'd0; 
            b3 = mult_op[3] ? {4'd0, op, 3'd0} : 86'd0; 
            b4 = mult_op[4] ? {3'd0, op, 4'd0} : 86'd0; 
            b5 = mult_op[5] ? {2'd0, op, 5'd0} : 86'd0; 
            b6 = mult_op[6] ? {1'd0, op, 6'd0} : 86'd0; 
            b7 = mult_op[7] ? {      op, 7'd0} : 86'd0;
            DIGIT_8_MULT = b0 ^ b1 ^ b2 ^ b3 ^ b4 ^ b5 ^ b6 ^ b7;
        end
    endfunction
endpackage;