`timescale 1ns / 1ps

module final_device #(
    parameter BAUD_RATE              = 9600,
    parameter NO_OF_TRANSFERRED_BITS  = 8,
    parameter CLK_F                   = 100000000
) (
    input RXD,
    output TXD,
    input clk,
    input reset
);

    // Definícia stavov FSM
    localparam [2:0] 
        ST_IDLE           = 3'd0,      // waiting for A0 or A1 byte from serial interface to start encryption communication. F0 - key is skipped; F1 - key data are expected
        ST_GET_PASS_LEN   = 3'd1,      // getting length of the key given by the user by serial interface
        ST_GET_PASS       = 3'd2,      // getting password of length given in state ST_GET_PASS_LEN
        ST_GET_PASS_VALID = 3'd3,      // waiting for key words from the key_gen module
        ST_GET_LEN        = 3'd4,      // getting length of the plain text given by the user by serial interface
        ST_GET_DATA       = 3'd5,      // getting plain text of length given in state ST_GET_LEN
        ST_PADDING        = 3'd6,      // padding last packet having user-given data with shorter length than encryption packet size
        ST_ENCRYPT        = 3'd7;      // data are given to the encryption module, either sends rx_state to read more, or prepare for next user prompt

    reg [2:0] rx_state; 
    
    localparam
        TX_WAIT = 1'd0,              // waits for the signal from rc6 encryption module for available data on it's output
        TX_SEND = 1'd1;              // sequentially sends data into the serial line, returning to TX_WAIT after full packet has been sent
        
    reg tx_state;
    

    // interface for RS232
    wire [7:0] rx_data;       // wire for incomming data
    wire       rx_strobe;     // signal for incoming data on rx_data
    reg  [7:0] tx_data;       // reg for sending data to the serial interface
    reg        tx_strobe;     // signal for interface that there are data ready to send
    wire       tx_ready;      // signal from serial interface if it's ready to send data

    // buffers and indexes for packets
    reg [7:0] packet_in [0:15];     // reg for data put into rc6 encryption module
    reg [3:0] packet_in_i;          // index reg for sequential data insertion into the packet_in reg
    
    reg [7:0] packet_out [0:15];    // reg for data taken out of the rc6 encryption module, to be sent by the serial interface
    reg [3:0] packet_out_i;         // index reg for sequential data insertion into the packet_out reg
    
    // interface for key_gen
    reg  valid_data;
    reg  start;                                  // reg to start the key schedule procedure
    reg  [7:0] key_gen_data;                     // reg for input of data into key_gen module
    wire [1407:0] key_words_wire;                // wire for output from the key_gen module
    wire key_valid;
    
    // interface for rc6
    reg  trigger;                                // reg for starting the encryption procedure
    reg  [1407:0] key_words_reg;                 // reg to store key words for encryption
    wire [31:0] A_out, B_out, C_out, D_out;      // wires for out from the rc6 encryption module
    wire valid_out;                              // signal that encryption module finished and there are valid data on it's output
    
    // key_gen/rc6 helper regs
    reg [7:0] key_len;                           // reg for length of user key, max relevant bytes = 176
    reg [7:0] pt_len;
    reg [7:0] pad;

    // module setups
    RS232 #(.BAUD_RATE(BAUD_RATE), .CLK_F(CLK_F)) serial_module (
        .RXD(RXD),
        .RXD_DATA(rx_data),
        .RXD_STROBE(rx_strobe),
        .TXD_DATA(tx_data),
        .TXD_STROBE(tx_strobe),
        .TXD(TXD),
        .TXD_READY(tx_ready),
        .CLK(clk),
        .RESET(reset)
    );
    
    key_gen kg(
        .clk(clk),
        .rst(reset),
        .start(start),
        .valid_data(valid_data),
        .data(key_gen_data),
        .key(key_words_wire),
        .valid(key_valid)
    );
    
    rc6 rc(
        .clk(clk),
        .reset(reset),
        .A_in({packet_in[3],  packet_in[2],  packet_in[1],  packet_in[0]}),
        .B_in({packet_in[7],  packet_in[6],  packet_in[5],  packet_in[4]}),
        .C_in({packet_in[11],  packet_in[10],  packet_in[9],  packet_in[8]}),
        .D_in({packet_in[15],  packet_in[14],  packet_in[13],  packet_in[12]}),
        .key(key_words_reg),
        .trigger(trigger),
        .A_out(A_out),
        .B_out(B_out),
        .C_out(C_out),
        .D_out(D_out),
        .valid(valid_out)
    );

    integer i;  // variable for descriptive for loops
    
    always @(posedge clk) begin
        if (reset) begin
            tx_state <= TX_WAIT;
            packet_out_i <= 0;
            
            rx_state <= ST_IDLE;
            packet_in_i <= 0;
            key_words_reg <= 1408'd0;
        end else begin
            case (rx_state)
                ST_IDLE: begin
                    trigger <= 0;
                    if (rx_strobe && rx_data == 8'hF0) begin
                        rx_state <= ST_GET_LEN;
                    end else if (rx_strobe && rx_data == 8'hF1) begin
                        rx_state <= ST_GET_PASS_LEN;
                    end 
                end
                
                ST_GET_PASS_LEN: begin
                    if (rx_strobe) begin
                        start <= 1;
                        key_gen_data <= rx_data;
                        key_len <= rx_data;
                        rx_state <= ST_GET_PASS;
                    end
                end
                
                ST_GET_PASS: begin
                    start <= 0;
                    valid_data <= 0;
                    if (rx_strobe) begin
                        valid_data <= 1;
                        key_gen_data <= rx_data;
                        rx_state <= (key_len == 8'd1) ? ST_GET_PASS_VALID : ST_GET_PASS;
                        key_len <= key_len - 8'd1;
                    end
                end
                
                ST_GET_PASS_VALID: begin
                    if (key_valid) begin
                        key_words_reg <= key_words_wire;
                        rx_state <= ST_GET_LEN;
                    end
                end

                ST_GET_LEN: begin
                    if (rx_strobe) begin
                        pt_len <= rx_data;
                        rx_state <= ST_GET_DATA;
                    end
                end

                ST_GET_DATA: begin
                    trigger <= 0;
                    if (rx_strobe) begin
                        packet_in[packet_in_i] <= rx_data;
                        if (packet_in_i == 4'd15) begin
                            rx_state <= ST_ENCRYPT;
                        end else if (pt_len == 8'd1) begin
                            pad <= 8'd15 - packet_in_i; // TODO
                            rx_state <= ST_PADDING;
                        end
                        packet_in_i <= packet_in_i + 4'd1;
                        pt_len <= pt_len - 8'd1;
                    end
                end

                ST_PADDING: begin
                    packet_in[packet_in_i] <= pad;
                    if (packet_in_i == 4'd15) begin
                        rx_state <= ST_ENCRYPT;
                    end
                    packet_in_i <= packet_in_i + 4'd1;
                end
                
                ST_ENCRYPT: begin
                    trigger <= 1;
                    if (pt_len == 0) begin
                        rx_state <= ST_IDLE;
                    end else begin
                        rx_state <= ST_GET_DATA;
                    end
                end
            endcase
            
            
            
            
            
            
            
            case (tx_state)
                TX_WAIT: begin
                    if (valid_out) begin
                        // Rozoberieme 32-bit slová spä? na bajty pre TX
                        {packet_out[3],  packet_out[2],  packet_out[1],  packet_out[0]}  <= A_out;
                        {packet_out[7],  packet_out[6],  packet_out[5],  packet_out[4]}  <= B_out;
                        {packet_out[11],  packet_out[10],  packet_out[9], packet_out[8]} <= C_out;
                        {packet_out[15], packet_out[14], packet_out[13], packet_out[12]} <= D_out;
                        tx_state <= TX_SEND;
                        packet_out_i <= 0;
                    end
                end

                TX_SEND: begin
                    if (tx_ready && !tx_strobe) begin
                        tx_strobe <= 1;
                        if (packet_out_i == 15) begin
                            tx_state <= TX_WAIT;
                        end
                        
                        tx_data <= packet_out[packet_out_i];
                        packet_out_i <= packet_out_i + 1;
                    end
                end
            endcase
            
            if (tx_strobe) begin
                tx_strobe <= 0;
            end
            
        end
    end

endmodule
