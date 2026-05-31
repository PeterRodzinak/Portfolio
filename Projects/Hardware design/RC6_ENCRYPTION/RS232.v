`timescale 1ns / 1ps

module RS232 #(
	parameter BAUD_RATE                          = 9600,
   parameter NO_OF_TRANSFERRED_BITS             = 8,

   parameter CLK_F                              = 50000000,                    // clock frequency of crystal (50 MHz)
   parameter BIT_INTERVAL_TIME                  = (CLK_F/BAUD_RATE)-1,
   parameter FIRST_BIT_INTERVAL_TIME            = (3*CLK_F)/(2*BAUD_RATE)
)(
	input RXD,														// RxD serial in
	output reg [NO_OF_TRANSFERRED_BITS-1:0] RXD_DATA,		// received data - parallel out
	output reg RXD_STROBE,										// high for 1 clock cycle upon data arrival

	input [NO_OF_TRANSFERRED_BITS-1:0] TXD_DATA,			// transmitted data - parallel in
	input TXD_STROBE,												// start of transmission
	output TXD,														// TxD serial out
	output reg TXD_READY,										// high when ready for transmission

	input CLK,
	input RESET
);
specify
	( CLK *> RXD_STROBE,TXD,TXD_READY ) = 1;
endspecify


   /////////////////////////////////////////////////////
	// RXD //////////////////////////////////////////////
   /////////////////////////////////////////////////////
	parameter RX_STATE_START=0,RX_STATE_INIT=1,RX_STATE_SEND=2,RX_STATE_ACK=3;
	parameter RX_STATE_W = 2;

   reg [15:0] RX_BIT_INTERVAL;
   reg RX_LOAD_BIT_INTERVAL;
   reg RX_LOAD_FIRST_BIT_INTERVAL;
   reg RX_EN_BIT_INTERVAL;
   wire RX_BIT_INTERVAL_END;

   reg [3:0] RX_BIT_COUNTER;
   reg RX_LOAD_BIT_COUNTER;
   reg RX_EN_BIT_COUNTER;
   wire RX_BIT_COUNTER_END;

   reg [NO_OF_TRANSFERRED_BITS-1:0] RXD_DATA_REG;
   reg RX_EN_DATA_OUT;

	reg [RX_STATE_W-1:0]	RX_STATE, RX_NEXT_STATE;

   // input synchronizer signals
   reg RXD_Q;
   reg RXD_QQ;

   /////////////////////////////////////////////////////
   // TXD //////////////////////////////////////////////
	/////////////////////////////////////////////////////
	parameter TX_STATE_START=0,TX_STATE_INIT=1,TX_STATE_SEND=2;
	parameter TX_STATE_W = 2;

   reg [15:0] TX_BIT_INTERVAL;
   reg TX_LOAD_BIT_INTERVAL;
   reg TX_EN_BIT_INTERVAL;
	wire TX_BIT_INTERVAL_END;

   reg [3:0] TX_BIT_COUNTER;
   reg TX_LOAD_BIT_COUNTER;
   reg TX_EN_BIT_COUNTER;
	wire TX_BIT_COUNTER_END;

   reg [NO_OF_TRANSFERRED_BITS+2:0] TXD_DATA_REG;

	reg [TX_STATE_W-1:0] TX_STATE, TX_NEXT_STATE;



   /////////////////////////////////////////////////////
   /////////////////////////////////////////////////////
	// RXD //////////////////////////////////////////////
   /////////////////////////////////////////////////////
   /////////////////////////////////////////////////////

   /////////////////////////////////////////////
	// INPUT SYNCHRONIZATION    /////////////////
   /////////////////////////////////////////////
   always @(posedge CLK) begin : RXD_SYNCHRONIZER
		RXD_Q  <= RXD;
		RXD_QQ <= RXD_Q;
   end
   

   /////////////////////////////////////////////
	// BIT INTERVAL MEASUREMENT /////////////////
   // measures time of one bit transfer in a BIT_RATE
   /////////////////////////////////////////////
   always @(posedge CLK) begin : RX_BIT_INTERVAL_MEASUREMENT
		if( RX_LOAD_BIT_INTERVAL == 1 )
			RX_BIT_INTERVAL <= BIT_INTERVAL_TIME;
		else if( RX_LOAD_FIRST_BIT_INTERVAL == 1 )
			RX_BIT_INTERVAL <= FIRST_BIT_INTERVAL_TIME;
		else if( RX_EN_BIT_INTERVAL == 1 )
			RX_BIT_INTERVAL <= RX_BIT_INTERVAL - 1;
   end
   
   assign RX_BIT_INTERVAL_END = ( RX_BIT_INTERVAL == 0 ) ? 1 : 0;


   /////////////////////////////////////////////
	// BIT COUNTING /////////////////////////////
   // counts bits to be send
   /////////////////////////////////////////////
   always @(posedge CLK) begin : RX_BIT_COUNTING
		if( RX_LOAD_BIT_COUNTER == 1 )
			RX_BIT_COUNTER <= NO_OF_TRANSFERRED_BITS;
		else if( RX_EN_BIT_COUNTER == 1 )
			RX_BIT_COUNTER <= RX_BIT_COUNTER - 1;
   end
   
   assign RX_BIT_COUNTER_END = ( RX_BIT_COUNTER == 0 ) ? 1 : 0;

   
   /////////////////////////////////////////////
	// FSM //////////////////////////////////////
	// controls RxD /////////////////////////////
   /////////////////////////////////////////////
   always @(RX_STATE, RXD_QQ, RX_BIT_INTERVAL_END, RX_BIT_COUNTER_END) begin : RX_FSM_TRANSITION_AND_OUTPUT
      RX_LOAD_BIT_COUNTER  <= 0;
      RX_EN_BIT_COUNTER    <= 0;
      RX_LOAD_BIT_INTERVAL <= 0;
      RX_LOAD_FIRST_BIT_INTERVAL <= 0;
      RX_EN_BIT_INTERVAL   <= 0;
      RXD_STROBE           <= 0;
      RX_EN_DATA_OUT       <= 0;
		
		// For synthesizer
		RX_NEXT_STATE <= RX_STATE_INIT;
   
      case( RX_STATE )
         RX_STATE_START		:	if( RXD_QQ == 0 ) begin
											RX_NEXT_STATE <= RX_STATE_INIT;
											/////////////////////
											RX_LOAD_BIT_COUNTER <= 1;
										end else
											RX_NEXT_STATE <= RX_STATE_START;
         RX_STATE_INIT	:	begin
													RX_NEXT_STATE <= RX_STATE_SEND;
													/////////////////////
													RX_LOAD_FIRST_BIT_INTERVAL <= 1;
												end
         RX_STATE_SEND		:	if( RX_BIT_INTERVAL_END == 0 ) begin
											RX_NEXT_STATE <= RX_STATE_SEND;
											/////////////////////
											RX_EN_BIT_INTERVAL <= 1;
										end else if( RX_BIT_COUNTER_END == 0 ) begin
											RX_NEXT_STATE <= RX_STATE_SEND;
											/////////////////////
											RX_LOAD_BIT_INTERVAL <= 1;
											RX_EN_BIT_COUNTER <= 1;
										end else begin
											RX_NEXT_STATE  <= RX_STATE_ACK;
											RX_EN_DATA_OUT <= 1;
										end
         RX_STATE_ACK		:	begin
											RX_NEXT_STATE <= RX_STATE_START;
											RXD_STROBE <= 1;
										end
			default:;
      endcase
   end

   always @(posedge CLK) begin : RX_FSM_REG
		if( RESET == 1 )
			RX_STATE <= RX_STATE_START;
		else
			RX_STATE <= RX_NEXT_STATE;
   end



   /////////////////////////////////////////////
	/// RXD shift register //////////////////////
   /////////////////////////////////////////////
   always @(posedge CLK) begin : RXD_SHIFT_REG
		if( RESET == 1 )
			RXD_DATA_REG <= 0;
		else if( RX_EN_BIT_COUNTER == 1 )
			RXD_DATA_REG <= { RXD_QQ , RXD_DATA_REG[NO_OF_TRANSFERRED_BITS-1:1] };
   end
   
   
   /////////////////////////////////////////////
	// RXD capture register /////////////////////
   /////////////////////////////////////////////
   always @(posedge CLK) begin : RXD_CAPTURE_REG
		if( RESET == 1 )
			RXD_DATA <= 0;
		else if( RX_EN_DATA_OUT == 1 )
			RXD_DATA <= RXD_DATA_REG;
   end




   /////////////////////////////////////////////////////
   /////////////////////////////////////////////////////
	// TXD //////////////////////////////////////////////
   /////////////////////////////////////////////////////
   /////////////////////////////////////////////////////




   /////////////////////////////////////////////
   // BIT INTERVAL MEASUREMENT /////////////////
   // measures time of one bit transfer in a BIT_RATE
   /////////////////////////////////////////////
   always @(posedge CLK) begin : TX_BIT_INTERVAL_MEASUREMENT
		if( TX_LOAD_BIT_INTERVAL == 1 )
			TX_BIT_INTERVAL <= BIT_INTERVAL_TIME;
		else if( TX_EN_BIT_INTERVAL == 1 )
			TX_BIT_INTERVAL <= TX_BIT_INTERVAL - 1;
   end
   
   assign TX_BIT_INTERVAL_END = ( TX_BIT_INTERVAL == 0 ) ? 1 : 0 ;


   /////////////////////////////////////////////
   // BIT COUNTING /////////////////////////////
   // counts bits to be send
   /////////////////////////////////////////////
   always @(posedge CLK) begin : TX_BIT_COUNTING
		if( TX_LOAD_BIT_COUNTER == 1 )
			TX_BIT_COUNTER <= NO_OF_TRANSFERRED_BITS+2;
		else if( TX_EN_BIT_COUNTER == 1 )
			TX_BIT_COUNTER <= TX_BIT_COUNTER - 1;
   end
   
   assign TX_BIT_COUNTER_END = ( TX_BIT_COUNTER == 0 ) ? 1 : 0 ;

   
   /////////////////////////////////////////////
   // FSM //////////////////////////////////////
   // controls TxD /////////////////////////////
   /////////////////////////////////////////////
   always @(TX_STATE, TXD_STROBE, TX_BIT_INTERVAL_END, TX_BIT_COUNTER_END) begin : TX_FSM_TRANSITION_AND_OUTPUT
      TX_LOAD_BIT_COUNTER  <= 0;
      TX_EN_BIT_COUNTER    <= 0;
      TX_LOAD_BIT_INTERVAL <= 0;
      TX_EN_BIT_INTERVAL   <= 0;
      TXD_READY            <= 0;
		
		// For synthesizer
		TX_NEXT_STATE <= TX_STATE_INIT;
   
      case( TX_STATE )
			TX_STATE_START		:	if( TXD_STROBE == 1 ) begin
											TX_NEXT_STATE <= TX_STATE_INIT;
											/////////////////////
											TXD_READY <= 1;
											TX_LOAD_BIT_COUNTER <= 1;
										end else begin
											TX_NEXT_STATE <= TX_STATE_START;
											/////////////////////
											TXD_READY <= 1;
										end
         TX_STATE_INIT	:	begin
													TX_NEXT_STATE <= TX_STATE_SEND;
													/////////////////////
													TX_LOAD_BIT_INTERVAL <= 1;
													TX_EN_BIT_COUNTER <= 1;
												end
         TX_STATE_SEND		:	if( TX_BIT_INTERVAL_END == 0 ) begin
											TX_NEXT_STATE <= TX_STATE_SEND;
											/////////////////////
											TX_EN_BIT_INTERVAL <= 1;
										end else if( TX_BIT_COUNTER_END == 0 ) begin
											TX_NEXT_STATE <= TX_STATE_SEND;
											/////////////////////
											TX_LOAD_BIT_INTERVAL <= 1;
											TX_EN_BIT_COUNTER <= 1;
										end else
											TX_NEXT_STATE <= TX_STATE_START;
			default:;
      endcase
   end

   always @(posedge CLK) begin : TX_FSM_REG
		if( RESET == 1 )
			TX_STATE <= TX_STATE_START;
		else
			TX_STATE <= TX_NEXT_STATE;
   end



   /////////////////////////////////////////////
   // TXD SHIFT REGISTER ///////////////////////
   /////////////////////////////////////////////
   always @(posedge CLK) begin : TX_SHIFT_REG
		if( RESET == 1 )
			TXD_DATA_REG <= ~0;
		else if( TX_LOAD_BIT_COUNTER == 1 ) begin
			TXD_DATA_REG[0]  <= 1;               // pre-start bit
			TXD_DATA_REG[1]  <= 0;               // start bit
			TXD_DATA_REG[NO_OF_TRANSFERRED_BITS+1:2] <= TXD_DATA;	// data bits
			TXD_DATA_REG[NO_OF_TRANSFERRED_BITS+2] <= 1;				// stop bit
		end else if( TX_EN_BIT_COUNTER == 1 )
			TXD_DATA_REG <= { 1'b1 , TXD_DATA_REG[10:1] };
   end
   
   
   assign TXD = TXD_DATA_REG[0];


endmodule
