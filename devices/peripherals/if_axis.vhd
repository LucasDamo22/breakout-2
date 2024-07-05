library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity if_axis is
	generic (
		SOC_SEGMENT : integer := 	16#e4#;		-- SoC segment address is 0xe4
		SOC_CLASS : integer := 		16#a9#;		-- Class address is 0xa9
		AXIS_DATA_WIDTH : integer := 8
	);
	port (
		-- external CPU interface signals
		addr_i : in std_logic_vector(31 downto 0);
		data_o : out std_logic_vector(31 downto 0);
		data_access_o : out std_logic;
		data_w_i: in std_logic;
		-- AXI stream interface signals
		axis_aclk_i : in std_logic;
		axis_aresetn_i : in std_logic;
		-- slave axi stream interface
		s_axis_tready_o : out std_logic;
		s_axis_tvalid_i : in std_logic;
		s_axis_tdata_i : in std_logic_vector(AXIS_DATA_WIDTH-1 downto 0)
	);
end if_axis;

architecture if_arch of if_axis is
	-- AXIS internal signals
	signal data_access : std_logic;
	signal data_read_axis : std_logic_vector(31 downto 0);
	signal s_axis_control, m_axis_control: std_logic_vector(1 downto 0);
	signal m_axis_data: std_logic_vector(AXIS_DATA_WIDTH-1 downto 0);
	
	-- custom logic signals (uart)
	signal device : std_logic_vector(15 downto 0);
begin
	s_axis_control(1) <= s_axis_tvalid_i;
	s_axis_tready_o <= s_axis_control(0);

	device <= addr_i(15 downto 0);
	data_access <= '1' when addr_i(31 downto 24) = std_logic_vector(to_unsigned(SOC_SEGMENT, 8)) and
				addr_i(23 downto 16) = std_logic_vector(to_unsigned(SOC_CLASS, 8))
				else '0';
	data_access_o <= data_access;
	data_o <= data_read_axis;
	
	
	-- AXI memory mapped registers
	
	-- Register read operations
	process (axis_aclk_i, axis_aresetn_i, device)
	begin
		if axis_aresetn_i = '0' then
			data_read_axis <= (others => '0');
			s_axis_control(0) <= '0';
		elsif axis_aclk_i'event and axis_aclk_i = '1' then
			if (data_access = '1') then				-- AXI peripheral is at 0xe4000000
				case device(6 downto 4) is
					when "001" =>		-- AXI control/status		0xe4000010 (RO)
						data_read_axis <= x"0000000" & "00" & s_axis_control(1) & s_axis_control(0);
					when "010" =>		-- AXI slave data (in)		0xe4000020 (RO)
						if s_axis_control(1) = '1' then
							data_read_axis <= x"000000" & s_axis_tdata_i;
							s_axis_control(0) <= '1';
						end if;
					when others =>
						data_read_axis <= (others => '0');
				end case;
			end if;
			
			-- AXI data valid, clear TREADY
			if (s_axis_control(1) = '1' and s_axis_control(0) = '1') then
				s_axis_control(0) <= '0';
			end if;
		end if;
	end process;

end if_arch;