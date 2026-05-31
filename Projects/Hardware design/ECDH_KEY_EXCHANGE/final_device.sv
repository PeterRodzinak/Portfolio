module final_device #(
    parameter BAUD_RATE               = 9600,
    parameter NO_OF_TRANSFERRED_BITS  = 8,
    parameter CLK_F                   = 100000000
) (
    input RXD,
    output TXD,
    input clk,
    input rst
);

    localparam [2:0] 
        RX_IDLE       = 3'd0,
        RX_KEY        = 3'd1,      
        RX_P_INF      = 3'd2,
        RX_P_X        = 3'd3,
        RX_P_Y        = 3'd4,
        RX_PROCESS    = 3'd5,
        RX_PROCESS2   = 3'd6;

    reg [2:0] rx_state; 
    
    localparam
        TX_IDLE = 1'd0,
        TX_SEND = 1'd1;
        
    reg tx_state;
    
    reg [7:0] key_ctr, p_x_ctr, p_y_ctr;
    reg [79:0] p_x, p_y, key, pres_x, pres_y;
    wire [79:0] res_x, res_y;
    reg p_inf, start;
    reg [78:0] lfsr;
    
    wire res_inf, valid;
    
    ECDH_CORE uut (
        .clk        (clk),
        .rst        (rst),
        .start      (start),
        
        .key        (key[78:0]),
        .p_x        (p_x[78:0]),
        .p_y        (p_y[78:0]),
        .p_inf      (p_inf),
        
        .res_x      (res_x[78:0]),
        .res_y      (res_y[78:0]),
        .res_inf    (res_inf),
        .valid      (valid)
    );
    
    reg [7:0] tx_data, tx_buffer_ctr;
    reg tx_strobe, tx_valid;
    reg [399:0] tx_buffer;
    wire [7:0] rx_data;
    wire rx_strobe, tx_ready;
    
    RS232 #(.BAUD_RATE(BAUD_RATE), .CLK_F(CLK_F)) serial_interface (
        .RXD(RXD),
        .RXD_DATA(rx_data),
        .RXD_STROBE(rx_strobe),
        .TXD_DATA(tx_data),
        .TXD_STROBE(tx_strobe),
        .TXD(TXD),
        .TXD_READY(tx_ready),
        .CLK(clk),
        .RESET(rst)
    );
    
    always @(posedge clk) begin
        if (rst) begin
            rx_state <= RX_IDLE;
            tx_state <= TX_IDLE;
            key_ctr <= 1;
            p_x_ctr <= 1;
            p_y_ctr <= 1;
            start <= 0;
            p_x <= 79'd0;
            p_y <= 79'd0;
            tx_strobe <= 0;
            lfsr <= 79'h1;
            tx_valid <= 0;
            pres_x <= 0;
            pres_y <= 0;
        end else begin
            lfsr <= {lfsr[77:0], lfsr[78] ^ lfsr[8]};
            case (rx_state)
                RX_IDLE: begin
                    start <= 0;
                    if (rx_strobe && rx_data == 8'hFF) begin
                        key_ctr <= 10;
                        rx_state <= RX_KEY;
                    end
                end
                RX_KEY: begin
                    key <= {1'd0, lfsr};
                    rx_state <= RX_P_INF;
                end
                RX_P_INF: begin
                    if (rx_strobe) begin
                        if (rx_data == 8'h00) begin
                            rx_state <= RX_P_X;
                            p_inf <= 0;
                            p_x_ctr <= 10;
                            p_y_ctr <= 10;
                        end else if (rx_data ==8'h01) begin
                            p_inf <= 1;
                            p_x <= 79'd0;
                            p_y <= 79'd0;
                            rx_state <= RX_PROCESS;
                        end
                    end
                end
                RX_P_X: begin
                    if (rx_strobe) begin
                        if (p_x_ctr == 1) begin
                            rx_state <= RX_P_Y;
                        end
                        p_x <= {p_x[71:0], rx_data};
                        p_x_ctr <= p_x_ctr - 1;
                    end
                end
                RX_P_Y: begin
                    if (rx_strobe) begin
                        if (p_y_ctr == 1) begin
                            rx_state <= RX_PROCESS;
                        end
                        p_y <= {p_y[71:0], rx_data};
                        p_y_ctr <= p_y_ctr - 1;
                    end
                end
                RX_PROCESS: begin
                    start <= 1;
                    rx_state <= RX_PROCESS2;
                end
                RX_PROCESS2: begin
                    start <= 0;
                    if (valid) begin
                        start <= 1;
                        pres_x <= res_x;
                        pres_y <= res_y;
                        p_x <= 79'h30CB127B63E42792F10F;
                        p_y <= 79'h547B2C88266BB04F713B;
                        rx_state <= RX_IDLE;
                    end
                end
            endcase
            
            case (tx_state)
                TX_IDLE: begin
                    tx_strobe <= 0;
                    if (valid) begin
                        if (tx_valid == 0) begin
                            tx_valid <= 1;
                        end else begin
                            tx_valid <= 0;
                            if (res_inf) begin
                                tx_strobe <= 1;
                                tx_data <= 8'h4f;
                            end else begin
                                tx_state <= TX_SEND;
                                tx_buffer_ctr <= 50;
                                tx_buffer <= {key, res_x, res_y, pres_x, pres_y};
                            end
                        end
                        
                    end
                end
                TX_SEND: begin
                    if (tx_strobe) begin
                        tx_strobe <= 0;
                    end else if (tx_ready) begin
                        if (tx_buffer_ctr == 1) begin
                            tx_state <= TX_IDLE;
                        end
                        tx_buffer_ctr <= tx_buffer_ctr - 1;
                        tx_data <= tx_buffer[399:392];
                        tx_buffer <= {tx_buffer[391:0], 8'd0};
                        tx_strobe <= 1;
                    end
                end
            endcase
        end
    end

endmodule
