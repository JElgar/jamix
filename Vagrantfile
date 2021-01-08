# Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
#
# Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
# which can be found via http://creativecommons.org (and should be included as 
# LICENSE.txt within the associated archive or repository).

Vagrant.configure( "2" ) do |config|
  # define base box
  config.vm.box = "ubuntu/bionic64"

  # configure SSH-based access
  config.ssh.forward_agent = true
  config.ssh.forward_x11   = true

  # configure shared folder between host and guest
  config.vm.synced_folder "./share", "/home/vagrant/share"

  # workaround for [1], which relates to a hard-coded path for log being
  # included in packaged box (which then fails to boot if/when said path
  # isn't valid).
  #
  # [1] https://github.com/hashicorp/vagrant/issues/9425
  config.vm.provider "virtualbox" do |vb|
    vb.customize [ 'modifyvm', :id, '--uartmode1', 'disconnected' ]
  end

  # provisioning step: general-purpose prologue
  config.vm.provision "prologue", type: "shell", preserve_order: true, run: "once", path: "provision_prologue.sh",
                                  env: { "UNIT_CODE" => "COMS20001", "UNIT_YEAR" => "2019", "UNIT_PATH" => "COMS20001_2019_TB-4" }
  # provisioning step: special-purpose wrt. unit
  config.vm.provision "unit",     type: "shell", preserve_order: true, run: "once", path: "provision_unit.sh",
                                  env: { "UNIT_CODE" => "COMS20001", "UNIT_YEAR" => "2019", "UNIT_PATH" => "COMS20001_2019_TB-4" }
  # provisioning step: general-purpose epilogue
  config.vm.provision "epilogue", type: "shell", preserve_order: true, run: "once", path: "provision_epilogue.sh",
                                  env: { "UNIT_CODE" => "COMS20001", "UNIT_YEAR" => "2019", "UNIT_PATH" => "COMS20001_2019_TB-4" }
end
