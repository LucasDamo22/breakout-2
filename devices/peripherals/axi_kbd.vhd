library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity axi_kbd is
	generic (
		DATA_WIDTH : integer := 8
	);
	port (
		-- external interface signals
		ps2_clk_i : in std_logic;
        ps2_data_i : in std_logic;
		-- AXI stream interface signals
		axis_aclk_i : in std_logic;
		axis_aresetn_i : in std_logic;
		-- master axi stream interface
		m_axis_tready_i : in std_logic;
		m_axis_tvalid_o : out  std_logic;
		m_axis_tdata_o : out  std_logic_vector(DATA_WIDTH-1 downto 0)
	);
end axi_kbd;

architecture axi_kbd_arch of axi_kbd is
	signal tvalid : std_logic;
    signal new_data_kbd : std_logic;
	type state_type is (st_idle, st_data, st_new_data);
	signal state : state_type;
	signal data : std_logic_vector(DATA_WIDTH-1 downto 0);
    signal data_reg : std_logic_vector(DATA_WIDTH-1 downto 0);

    component ps2_keyboard
        generic (
            clk_freq : integer := 50_000_000
            debounce_counter_size : integer := 8
            );
        port (
            clk : in std_logic;
            ps2_clk : in std_logic;
            ps2_data : out std_logic;
            ps2_code_new : out std_logic;
            ps2_code : out std_logic_vector(7 downto 0)
        );
    end component;
begin
	m_axis_tvalid_o <= tvalid;
	

	kbd_process: process(axis_aclk_i) is
	begin
		if axis_aresetn_i = '0' then
			tvalid <= '0';
		elsif axis_aclk_i'event and axis_aclk_i = '1' then
			case state is
				when st_idle =>
					tvalid <= '1';
					if new_data_kbd = '1' then
                        state <= st_new_data;

                    elsif m_axis_tready_i = '1' then
						m_axis_tdata_o <= data_reg;
						state <= st_data;
					end if;

				when st_data =>
					tvalid <= '0';
					state <= st_idle;
                
                when st_new_data =>
                    tvalid <= '0';
                    data_reg <= data;
                    state <= st_idle;
                    
			end case;
		end if;
	end process;

    kbd: ps2_keyboard
        generic map (
            clk_freq => 50_000_000,
            debounce_counter_size => 4
        )
        port map (
            clk => axis_aclk_i,
            ps2_clk => ps2_clk_i,
            ps2_data => ps2_data_i,
            ps2_code_new => new_data_kbd,
            ps2_code => data
        );

end axi_kbd_arch;